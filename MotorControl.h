#pragma once
#include <Arduino.h>

// Pin definitions
#define ENA_PIN 25  // Enable pin for motor driver
#define IN1_PIN 32  // Input 1 for motor driver
#define IN2_PIN 33  // Input 2 for motor driver
#define ESTOP_PIN 26  // Emergency stop input
#define MAG_SENSOR_PIN 27  // Magnetic sensor input

// Motor speed and position constants
#define MOTOR_SPEED 255  // Default motor speed (0-255)
#define REV_FULL 10  // Revolutions for full raised position
#define REV_HALF 5   // Revolutions for half raised position
#define REV_DOWN 0    // Revolutions for fully lowered position

// Function declarations
void setupMotor();
void setMotorForward(uint8_t speed);
void setMotorReverse(uint8_t speed);
void stopMotor();
void checkEStop();
void moveToPosition(String position);
void updateMotorMovement();
void resetMotorPosition();
int getCurrentPosition();
bool isMoving();
String getMotorStatus();

// Motor state variables
extern volatile int revCounter;
extern bool moving;
extern bool moveUp;
extern int currentTarget;
extern int currentPWM;          // current PWM duty
extern int targetPWM;           // where we want to ramp to
extern unsigned long lastPWMUpdate; // last time we updated PWM
extern bool softStarting;       // whether we are in soft start phase