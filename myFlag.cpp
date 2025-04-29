#include <ArduinoJson.h>
#include "myFlag.h"
#include "flagpos.h"
#include "MotorControl.h"  // Needed for setFlagPosition()
#include <WiFi.h>          // Required for WiFiServer and WiFiClient

extern WiFiServer server;  // Use WiFiServer declared in FlagSentry.ino or globals.h

// === POST Handler for /setFlagPosition ===
void handleSetFlagPosition() {
    WiFiClient client = server.accept();
    if (!client) return;

    String request = client.readStringUntil('\r');
    client.read(); // Consume the newline character
    String body = "";

    // Wait for empty line then read body
    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            body = client.readString();
            break;
        }
    }

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        client.println("HTTP/1.1 400 Bad Request");
        client.println("Content-Type: text/plain");
        client.println("Connection: close");
        client.println();
        client.println("Invalid JSON");
        client.stop();
        return;
    }

    String incomingPosition = doc["position"];

    if (incomingPosition == "Full") {
        setFlagPosition(FLAG_FULL);
    }
    else if (incomingPosition == "Half") {
        setFlagPosition(FLAG_HALF);
    }
    else if (incomingPosition == "Down") {
        setFlagPosition(FLAG_DOWN);
    }
    else if (incomingPosition == "Auto") {
        setFlagPosition(FLAG_AUTO);
    }
    else {
        setFlagPosition(FLAG_UNKNOWN);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Flag Position Updated");
    client.stop();
}

// === HTML Page Handler (called manually in loop if MP3 not playing) ===
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
                     const char* (*flagToString)(FlagPosition)) {

    WiFiClient client = server.accept();
    if (!client) return;

    String request = client.readStringUntil('\r');
    client.read(); // Consume newline

    Serial.println("Incoming request:");
    Serial.println(request);

    // If request is for setting flag position, defer to JSON handler
    if (request.indexOf("POST /setFlagPosition") >= 0) {
        handleSetFlagPosition();  // re-use the POST handler above
        return;
    }

    // Otherwise, serve the HTML page
    String html = htmlTemplate;
    html.replace("{TIME}", timeStr);
    html.replace("{SERIAL}", deviceSerial);
    html.replace("{TEMP}", String(tempC, 2));
    html.replace("{TEMP_F}", String(tempF, 2));
    html.replace("{PRESSURE}", String(pressure, 2));
    html.replace("{LIGHT}", String(lightVal));
    html.replace("{LIGHT_COND}", lightCond);
    html.replace("{FLAG_STATUS}", flagToString(flagStatus));

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.print(html);
    client.stop();

    Serial.println("HTML page served.");
}
