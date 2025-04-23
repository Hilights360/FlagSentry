 // globals.h
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "flagpos.h"

extern WiFiServer server;

// Shared device identity
extern String deviceSerial;

// Cached sensor values
extern float cachedTempC;
extern float cachedTempF;
extern float cachedPressure;
extern int cachedLightVal;
extern String cachedLightCond;
extern String cachedTimeStr;
extern String cachedHTML;

// Flag control
extern FlagPosition currentFlagStatus;
extern FlagPosition targetFlagStatus;
extern bool autoModeEnabled;

// URLs
extern String flagStatusUrl;
extern String postDataUrl;

// Timing intervals
extern unsigned long lastLogTime;
extern unsigned long lastUploadTime;
extern unsigned long lastDebugPrintTime;
extern unsigned long lastPageRefreshTime;

// Globals for SD card
extern bool sdCardInitialized;
