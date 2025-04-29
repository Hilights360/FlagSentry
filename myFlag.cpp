#include <WiFi.h>   // <-- ADD THIS LINE
#include "myFlag.h"
#include "flagpos.h"
#include <ArduinoJson.h> // <-- Also needed for JSON parsing


// Assume `server`, `cachedHTML`, `cachedTimeStr`, etc., are defined as `extern` in Globals.h
void serveMyFlagHTML(NetworkServer& server,
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
    if (client) {
        String request = client.readStringUntil('\r');
        Serial.println("Incoming HTTP request:");
        Serial.println(request);

        // Check if the POST request contains flag position update
        if (request.indexOf("POST /setFlagPosition") >= 0) {
            // Parse body to extract the flag position
            String body = client.readStringUntil('\r');
            if (body.indexOf("\"position\":\"Full\"") >= 0) {
                flagStatus = FLAG_FULL;
            } else if (body.indexOf("\"position\":\"Half\"") >= 0) {
                flagStatus = FLAG_HALF;
            } else if (body.indexOf("\"position\":\"Down\"") >= 0) {
                flagStatus = FLAG_DOWN;
            } else if (body.indexOf("\"position\":\"Auto\"") >= 0) {
                flagStatus = FLAG_AUTO;    
            }

            // Send a plain text response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("Flag Position Updated Successfully");
            client.stop();
            return;
        }

        // Serve the HTML page if the request is not for /setFlagPosition
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
        Serial.println("myFlag page served to client.");
        client.stop();
    }
}
void handleSetFlagPosition() {
    if (server.hasArg("plain") == false) {
        server.send(400, "text/plain", "Bad Request: Body Missing");
        return;
    }

    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Bad Request: Invalid JSON");
        return;
    }

    String incomingPosition = doc["position"];

    if (incomingPosition == "Full") {
        setFlagPosition(FULL);
        server.send(200, "text/plain", "Flag set to Full");
    }
    else if (incomingPosition == "Half") {
        setFlagPosition(HALF);
        server.send(200, "text/plain", "Flag set to Half");
    }
    else if (incomingPosition == "Down") {
        setFlagPosition(DOWN);
        server.send(200, "text/plain", "Flag set to Down");
    }
    else if (incomingPosition == "Auto") {
        setFlagPosition(AUTO);  // Auto mode - you must define this behavior
        server.send(200, "text/plain", "Flag set to Auto Mode");
    }
    else {
        server.send(400, "text/plain", "Invalid flag position received");
    }
}
