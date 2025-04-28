#include <Arduino.h>
#include "MotorControl.h"
#include "esp32-hal-ledc.h"  // <-- ADD THIS!

// Motor Control Variables
bool moving = false;
int targetRevolutions = 0;
int revolutions = 0;
unsigned long lastMagSensorCheck = 0;
bool lastMagSensorState = false;
String currentStatus = "Idle";

// *** Added for Soft Start ***
int currentPWM = 0;
int targetPWM = MOTOR_SPEED;
unsigned long lastPWMUpdate = 0;
bool softStarting = false;

void setupMotor() {
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    pinMode(MAG_SENSOR_PIN, INPUT_PULLUP);

    // PWM setup for ENA_PIN
    ledcSetup(0, 5000, 8);       // PWM Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(ENA_PIN, 0);   // Attach ENA_PIN to PWM Channel 0

    stopMotor();
    resetMotorPosition();
}

void setMotorForward(uint8_t speed) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    ledcWrite(0, speed);
    currentStatus = "Moving Forward";
}

void setMotorReverse(uint8_t speed) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
    ledcWrite(0, speed);
    currentStatus = "Moving Reverse";
}

void stopMotor() {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    ledcWrite(0, 0);
    moving = false;
    softStarting = false; // *** Added for Soft Start ***
    currentStatus = "Stopped";
}

void checkEStop() {
    if (digitalRead(ESTOP_PIN) == LOW) {
        stopMotor();
        Serial.println("!!! Emergency Stop Activated !!!");
    }
}

void moveToPosition(String position) {
    if (position == "FULL") {
        targetRevolutions = REV_FULL;
    } else if (position == "HALF") {
        targetRevolutions = REV_HALF;
    } else if (position == "DOWN") {
        targetRevolutions = REV_DOWN;
    } else {
        Serial.println("Unknown Position Requested");
        return;
    }

    if (targetRevolutions > revolutions) {
        setMotorForward(0); // Start with 0 speed for soft start
    } else if (targetRevolutions < revolutions) {
        setMotorReverse(0); // Start with 0 speed for soft start
    } else {
        Serial.println("Already at Target Position");
        return;
    }

    moving = true;
    softStarting = true;            // *** Added for Soft Start ***
    currentPWM = 0;                 // Start ramp from 0
    targetPWM = MOTOR_SPEED;        // Ramp target
    lastPWMUpdate = millis();       // Initialize timer
}

void updateMotorMovement() {
    if (!moving) return;

    // *** Soft Start PWM Ramping ***
    if (softStarting) {
        unsigned long now = millis();
        if (now - lastPWMUpdate >= 20) { // Update every 20ms
            lastPWMUpdate = now;
            currentPWM += 5; // Increase PWM slowly
            if (currentPWM >= targetPWM) {
                currentPWM = targetPWM;
                softStarting = false;
            }
            ledcWrite(0, currentPWM); // <-- Critical! Update PWM output here
        }
    }

    // Update motor revolutions
    bool magSensorState = digitalRead(MAG_SENSOR_PIN) == LOW;
    if (magSensorState && !lastMagSensorState) {
        revolutions++;
        Serial.printf("Revolutions: %d\n", revolutions);
    }
    lastMagSensorState = magSensorState;

    // Check if target reached
    if (revolutions >= targetRevolutions) {
        stopMotor();
    }
}

void resetMotorPosition() {
    revolutions = 0;
}

int getCurrentPosition() {
    return revolutions;
}

bool isMoving() {
    return moving;
}

String getMotorStatus() {
    return currentStatus;
}
