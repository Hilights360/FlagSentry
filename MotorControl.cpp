#include "MotorControl.h"

int currentPWM = 0;
int targetPWM = MOTOR_SPEED;
unsigned long lastPWMUpdate = 0;
bool softStarting = false;


// Global variables for motor control
volatile int revCounter = 0;
bool moving = false;
bool moveUp = true;
int currentTarget = 0;
unsigned long lastMotorUpdate = 0;
const unsigned long MOTOR_UPDATE_INTERVAL = 100; // Check motor status every 100ms

// Interrupt handler for magnetic sensor pulses
void IRAM_ATTR handleMagPulse() {
    // Fix for volatile warning
    int currentCount = revCounter;
    revCounter = currentCount + 1;
}

void setupMotor() {
    // Configure motor control pins
    pinMode(ENA_PIN, OUTPUT);
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    pinMode(MAG_SENSOR_PIN, INPUT_PULLUP);

    // Attach interrupt for magnetic sensor
    attachInterrupt(digitalPinToInterrupt(MAG_SENSOR_PIN), handleMagPulse, RISING);
    
    //Setup Softstart
    currentPWM = 0;
    targetPWM = MOTOR_SPEED;
    lastPWMUpdate = 0;
    softStarting = false;

    // Initialize motor in stopped state
    stopMotor();
    Serial.println("✅ Motor setup complete");
    
    // Initial position report
    Serial.printf("Initial position: %d revolutions\n", revCounter);
}

void setMotorForward(uint8_t speed) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, speed);
    Serial.printf("🔼 Motor moving forward at speed %d\n", speed);
}

void setMotorReverse(uint8_t speed) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    analogWrite(ENA_PIN, speed);
    Serial.printf("🔽 Motor moving reverse at speed %d\n", speed);
}

void stopMotor() {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, 0);
    moving = false;
    Serial.println("⏹️ Motor stopped");
}

void checkEStop() {
    if (digitalRead(ESTOP_PIN) == LOW) {
        stopMotor();
        moving = false;
        Serial.println("🛑 E-STOP Triggered!");
    }
}

void moveToPosition(String position) {
    // Don't start a new movement if already moving
    if (moving) {
        Serial.println("⚠️ Motor already moving, command ignored");
        return;
    }
    //Setup Softstart
    currentPWM = 0;
    targetPWM = MOTOR_SPEED;
    softStarting = true;

    // Calculate target position
    int newTarget;
    if (position == "FULL") {
        newTarget = REV_FULL;
    } else if (position == "HALF") {
        newTarget = REV_HALF;
    } else if (position == "DOWN") {
        newTarget = REV_DOWN;
    } else {
        Serial.printf("❌ Invalid position command: %s\n", position.c_str());
        return;
    }

    // Set movement parameters
    currentTarget = newTarget;
    moveUp = (revCounter < currentTarget);
    moving = true;

    // Start motor movement
    if (moveUp) {
        setMotorForward(MOTOR_SPEED);
    } else {
        setMotorReverse(MOTOR_SPEED);
    }

    // Log movement details
    Serial.printf("🎯 Moving to %s position (Target: %d revs, Current: %d revs, Direction: %s)\n", 
                 position.c_str(), currentTarget, revCounter, moveUp ? "UP" : "DOWN");
}

void updateMotorMovement() {
    unsigned long currentMillis = millis();
    
    // Only update at specified intervals
    if (currentMillis - lastMotorUpdate < MOTOR_UPDATE_INTERVAL) {
        return;
    }
    lastMotorUpdate = currentMillis;

    //Check if there has been a softstart
    if (softStarting) {
    unsigned long now = millis();
    if (now - lastPWMUpdate > 20) { // ramp every 20ms
        lastPWMUpdate = now;
        currentPWM += 5; // ramp by 5 each time
        if (currentPWM >= targetPWM) {
            currentPWM = targetPWM;
            softStarting = false;
        }
        // Write the ramped PWM to motor
        ledcWrite(0, currentPWM); // Assuming channel 0 for ENA
    }
}

    // If not moving, nothing to update
    if (!moving) {
        return;
    }

    // Check emergency stop first
    checkEStop();
    if (!moving) { // If E-stop triggered, exit
        return;
    }

    // Check if we've reached the target
    bool reachedTarget = false;
    if (moveUp && revCounter >= currentTarget) {
        reachedTarget = true;
        Serial.println("✅ Reached upper target position");
    }
    else if (!moveUp && revCounter <= currentTarget) {
        reachedTarget = true;
        Serial.println("✅ Reached lower target position");
    }

    // Stop motor if target reached
    if (reachedTarget) {
        stopMotor();
        Serial.printf("📍 Final position: %d revolutions\n", revCounter);
    }
}

int getCurrentPosition() {
    return revCounter;
}

bool isMoving() {
    return moving;
}

String getMotorStatus() {
    char status[100];
    snprintf(status, sizeof(status), 
             "Moving: %s, Direction: %s, Position: %d, Target: %d", 
             moving ? "YES" : "NO",
             moveUp ? "UP" : "DOWN",
             revCounter,
             currentTarget);
    return String(status);
}