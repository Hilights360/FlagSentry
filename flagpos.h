#ifndef FLAGPOS_H
#define FLAGPOS_H
#include "MotorControl.h"
#include <Arduino.h>

// Define possible flag positions as an enum.
enum FlagPosition {
    FLAG_FULL,
    FLAG_HALF,
    FLAG_DOWN,
    FLAG_STOW,
    FLAG_AUTO,    
    FLAG_UNKNOWN
};

// Convert enum to string.
inline const char* flagPosToString(FlagPosition pos) {
    switch (pos) {
        case FLAG_FULL:   return "FULL";
        case FLAG_HALF:   return "HALF";
        case FLAG_DOWN:   return "DOWN";
        case FLAG_STOW:   return "STOW";
        case FLAG_AUTO:   return "AUTO";
        default:          return "UNKNOWN";
    }
}

// Convert string to enum.
inline FlagPosition stringToFlagPos(String str) {
    str.trim();
    str.toUpperCase();
    if (str == "FULL") {
        return FLAG_FULL;
    } else if (str == "HALF") {
        return FLAG_HALF;
    } else if (str == "DOWN") {
        return FLAG_DOWN;
    } else if (str == "STOW") {
        return FLAG_STOW;
    } else if (str == "AUTO") {  // Add this condition
        return FLAG_AUTO;    
    } else {
        return FLAG_UNKNOWN;
    }
}

// Global variables for flag positions.
extern FlagPosition currentFlagStatus;         // Measured/last known flag position.
extern FlagPosition targetFlagPosition;          // Desired flag position.
extern FlagPosition lastCommandedFlagPosition;   // Last commanded flag position.

// New: manual override flag and last photocell value.
extern bool manualOverrideActive;
extern int lastPhotocellValue;

// Update target position from Serial input.
void updateTargetPositionFromSerial();

// Update target position from HTTP input.
void updateTargetPositionFromHTTP(String httpInput);

// Update target position based on a photocell reading.
// This function will clear the manual override if the reading changes significantly.
void updateTargetPositionFromPhotocell(int lightValue);

// Apply the target position by commanding the motor (if needed).
void applyTargetPosition();

#endif // FLAGPOS_H
