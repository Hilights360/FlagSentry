#pragma once

#include <Arduino.h>
#include "flagpos.h"

// === Motor Control Pins ===
#define ENA_PIN 25    // PWM output to motor driver (ENA)
#define IN1_PIN 32    // Motor driver input 1
#define IN2_PIN 33    // Motor driver input 2
#define ESTOP_PIN 26  // Emergency stop input
#define MAG_SENSOR_PIN 27 // Magnetic sensor input

// === Motor Tuning Variables ===
// *** Added for adjustable soft start control ***
extern int startPWM;  // Starting PWM value for motor ramp (0-255)
extern int targetPWM; // Final PWM target speed (0-255)
extern int rampTime;  // Time to ramp from startPWM to targetPWM (milliseconds)

// === Flag Position Settings (Adjustable) ===
// *** Changed from #define to variables for dynamic adjustment ***
extern int fullPositionRevolutions;  // Number of revolutions for FULL flag
extern int halfPositionRevolutions;  // Number of revolutions for HALF mast
extern int downPositionRevolutions;  // Number of revolutions for DOWN

// === Function Declarations ===
void setupMotor();
void setMotorForward(uint8_t speed);
void setMotorReverse(uint8_t speed);
void stopMotor();
void moveToPosition(String position);
void updateMotorMovement();
void resetMotorPosition();
int getCurrentPosition();
bool isMoving();
String getMotorStatus();
void checkEStop();
