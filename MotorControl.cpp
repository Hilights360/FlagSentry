#include "MotorControl.h"

// === Motor Control Variables ===
bool moving = false;
volatile int revolutions = 0; // *** Changed to volatile for interrupt-safe counting ***
int targetRevolutions = 0;
int currentPWM = 0;
bool softStarting = false;
unsigned long lastPWMUpdate = 0;
String currentStatus = "Idle";

// === Motor Tuning Settings (Adjustable) ===
// *** Added for adjustable soft start control ***
int startPWM = 50;               // Starting PWM value (out of 255)
int targetPWM = 255;              // Full-speed PWM value
int rampTime = 1000;              // Ramp time in ms

// === Flag Position Variables ===
// *** Changed from #define to adjustable variables ***
int fullPositionRevolutions = 10;  // Default full flag revolutions
int halfPositionRevolutions = 5;   // Default half mast revolutions
int downPositionRevolutions = 0;   // Down position revolutions

// === Interrupt Service Routine for Magnetic Sensor ===
void IRAM_ATTR magnetPulseISR() {
    revolutions++;
}

// === Setup Motor ===
void setupMotor() {
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    pinMode(MAG_SENSOR_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(MAG_SENSOR_PIN), magnetPulseISR, FALLING); // *** Added interrupt for revolution counting ***

    ledcSetup(0, 5000, 8);  // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(ENA_PIN, 0);  // Attach ENA pin to PWM Channel 0

    stopMotor();
    resetMotorPosition();
}

// === Set Motor Forward ===
void setMotorForward(uint8_t speed) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    ledcWrite(0, speed);
}

// === Set Motor Reverse ===
void setMotorReverse(uint8_t speed) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    ledcWrite(0, speed);
}

// === Stop Motor ===
void stopMotor() {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    ledcWrite(0, 0);
    moving = false;
    currentStatus = "Idle";
}

// === Move to Flag Position ===
void moveToPosition(String position) {
    resetMotorPosition();
    moving = true;
    softStarting = true;
    currentPWM = startPWM; // *** Start from configured start PWM ***

    if (position == "FULL") {
        targetRevolutions = fullPositionRevolutions;
        setMotorForward(currentPWM);
    } else if (position == "HALF") {
        targetRevolutions = halfPositionRevolutions;
        setMotorForward(currentPWM);
    } else if (position == "DOWN") {
        targetRevolutions = downPositionRevolutions;
        setMotorReverse(currentPWM);
    }
    currentStatus = "Moving " + position;
}

// === Update Motor Movement ===
void updateMotorMovement() {
    if (!moving) return;

    // === Soft Start PWM Ramping ===
    if (softStarting) {
        unsigned long now = millis();
        if (now - lastPWMUpdate >= 10) { // Update every 10ms
            lastPWMUpdate = now;
            int pwmStep = (targetPWM - startPWM) * 10 / rampTime; // *** Auto calculate step based on ramp time ***
            if (pwmStep < 1) pwmStep = 1; // Minimum step of 1 to avoid freezing
            currentPWM += pwmStep;
            if (currentPWM >= targetPWM) {
                currentPWM = targetPWM;
                softStarting = false;
            }
            ledcWrite(0, currentPWM);
        }
    }

    // === Target Revolution Check ===
    if (revolutions >= targetRevolutions) {
        stopMotor();
    }
}

// === Reset Motor Position ===
void resetMotorPosition() {
    revolutions = 0;
}

// === Get Current Motor Position ===
int getCurrentPosition() {
    return revolutions;
}

// === Check if Motor is Moving ===
bool isMoving() {
    return moving;
}

// === Get Motor Status ===
String getMotorStatus() {
    return currentStatus;
}

// === Check Emergency Stop ===
void checkEStop() {
    if (digitalRead(ESTOP_PIN) == LOW) {
        stopMotor();
        Serial.println("Emergency Stop Triggered!");
    }
}
