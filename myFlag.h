#pragma once

#include <WiFi.h>
#include <ArduinoJson.h> // <-- Also needed for JSON parsing
#include "flagpos.h"
#include "globals.h" // Include any global shared variables like cachedHTML etc.

typedef WiFiServer NetworkServer;


void setFlagPosition(FlagPosition position);

void serveMyFlagHTML(WiFiServer& server,
                     const String& htmlTemplate,
                     const String& timeStr,
                     const String& deviceSerial,
                     float tempC,
                     float tempF,
                     float pressure,
                     int lightVal,
                     const String& lightCond,
                     FlagPosition flagStatus,
                     const char* (*flagToString)(FlagPosition));

