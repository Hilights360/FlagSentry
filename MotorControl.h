#pragma once

#include <Arduino.h>

// Motor driver pins
#define ENA_PIN 25
#define IN1_PIN 32
#define IN2_PIN 33

// Emergency Stop and Magnetic Sensor pins
#define ESTOP_PIN 26
#define MAG_SENSOR_PIN 27

// Motor Constants
#define MOTOR_SPEED 255      // Maximum motor speed (PWM duty)
#define REV_FULL 10          // Full flag position in revolutions
#define REV_HALF 5           // Half flag position in revolutions
#define REV_DOWN 0           // Down flag position in revolutions

// Motor control functions
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

// *** Added for Soft Start ***
extern int currentPWM;              // Current PWM duty during soft start
extern int targetPWM;               // Target PWM duty to reach
extern unsigned long lastPWMUpdate; // Last millis() when PWM was updated
extern bool softStarting;           // Whether motor is in soft start ramp
