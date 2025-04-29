#pragma once

#include <WiFi.h>
#include <ArduinoJson.h> // <-- Also needed for JSON parsing
#include "flagpos.h"
#include "globals.h" // Include any global shared variables like cachedHTML etc.

typedef WiFiServer NetworkServer;

void setFlagPosition(FlagPosition position);

void serveMyFlagHTML(WebServer& server,
                     const String& htmlTemplate,
                     const String& deviceSerial,
                     const String& timeStr,
                     float tempC,
                     float tempF,
                     float pressure,
                     int lightVal,
                     const String& lightCond,
                     FlagPosition flagStatus,
                     const char* (*flagToString)(FlagPosition));
