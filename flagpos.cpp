#include "flagpos.h"
#include "MotorControl.h" 

// Initialize global variables.
FlagPosition currentFlagStatus = FLAG_UNKNOWN;
FlagPosition targetFlagPosition = FLAG_UNKNOWN;
FlagPosition lastCommandedFlagPosition = FLAG_UNKNOWN;

bool manualOverrideActive = false;
int lastPhotocellValue = -1;  // Start with an invalid reading

void updateTargetPositionFromSerial() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    FlagPosition newTarget = stringToFlagPos(input);
    if (newTarget != FLAG_UNKNOWN) {
      targetFlagPosition = newTarget;
      manualOverrideActive = true;  // A manual command sets the override.
      Serial.print("Serial set target to: ");
      Serial.println(flagPosToString(targetFlagPosition));
    } else {
      Serial.println("Invalid input from Serial.");
    }
  }
}

void updateTargetPositionFromHTTP(String httpInput) {
  FlagPosition newTarget = stringToFlagPos(httpInput);
  if (newTarget != FLAG_UNKNOWN) {
    targetFlagPosition = newTarget;
    manualOverrideActive = true;  // A manual command sets the override.
    Serial.print("HTTP set target to: ");
    Serial.println(flagPosToString(targetFlagPosition));
  } else {
    Serial.println("Invalid HTTP input.");
  }
}

void updateTargetPositionFromPhotocell(int lightValue) {
  const int threshold = 200;  // Change threshold as needed.
  
  // If this is the first photocell reading, record it.
  Serial.print("Light value: ");
  Serial.print(lightValue);
  Serial.print(", Auto mode: ");
  Serial.println(targetFlagPosition == FLAG_AUTO ? "YES" : "NO");
  if (lastPhotocellValue < 0) {
    lastPhotocellValue = lightValue;
  }
  
  // Check if conditions have changed significantly.
  if (abs(lightValue - lastPhotocellValue) > threshold) {
    // Clear manual override on significant photocell change.
    if (manualOverrideActive) {
      Serial.println("Photocell change detected - clearing manual override.");
      manualOverrideActive = false;
    }
    lastPhotocellValue = lightValue;
  }
  
  // If no manual override is active, update the target automatically.
  if (!manualOverrideActive) {
    FlagPosition autoTarget;
    // Example logic: lower flag when dark, raise when light.
    if (lightValue < 1000) {
      autoTarget = FLAG_DOWN;
    } else {
      autoTarget = FLAG_FULL;
    }
    // Optionally, you can incorporate a "HALF" condition if desired.
    if (targetFlagPosition != autoTarget) {
      targetFlagPosition = autoTarget;
      Serial.print("Photocell automatically set target to: ");
      Serial.println(flagPosToString(targetFlagPosition));
    }
  }
}

void applyTargetPosition() {
  if (targetFlagPosition != lastCommandedFlagPosition) {
    Serial.println("applyTargetPosition called");
    lastCommandedFlagPosition = targetFlagPosition;
    Serial.print("Applying new target position: ");
    Serial.println(flagPosToString(targetFlagPosition));
    // Call your motor control function here.
    // For example:
    moveToPosition(String(flagPosToString(targetFlagPosition)));
    // Once movement is complete, update current flag status:
    currentFlagStatus = targetFlagPosition;
  }
}
