/*Flag Sentry V1
Code for full autonomus flag control with WIFI support
by Tim Nash 2025
Updates 4/23/25*/
// TODO: Implement WiFi tethering functionality for the ESP32 as a server.

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include <time.h>
#include <ESPmDNS.h>
#include "myFlag.h" //Rotines for updating myFlag.local
#include "MotorControl.h" // Include motor control and Flag position headers.
#include "flagpos.h" //Flag position Main.
#include "esp_heap_caps.h" // Include this header for heap_caps_get_free_size
#include "MP3Player.h"

WiFiServer server(80);

#ifndef NDEBUG
#define NDEBUG
#endif
// === Pin Definitions for sensors ===
#define PHOTOCELL_PIN 36   // ADC input for the photocell
#define SDA_PIN 21         // BMP280 Sensor Pins I2C SDA
#define SCL_PIN 22         // BMP280 Sensor Pins I2C SCL
#define SD_CS_PIN 5        // SD card chip select
//default MISO_PIN 19
//default MOSI_PIN 23
//default SCK_PIN 18

/*Pins in other defs:
#define ENA_PIN 25  // Enable pin for motor driver
#define IN1_PIN 32  // Input 1 for motor driver
#define IN2_PIN 33  // Input 2 for motor driver
#define ESTOP_PIN 26  // Emergency stop input
#define MAG_SENSOR_PIN 27  // Magnetic sensor input
#define I2S_DOUT_PIN 04  // Data Out (DIN on MAX98357 AUDIO)
#define I2S_BCLK_PIN 14  // Bit Clock (BCLK on MAX98357 AUDIO)
#define I2S_LRCLK_PIN 13 // Left/Right Clock (LRCLK on MAX98357 AUDIO)
*/

// === WiFi Credentials and Server URLs ===
const char* ssid = "COILighting";
const char* password = "@@CoIIoT";
const char* dataUploadUrl = "http://flagsentry.com/save_data.php";
const char* flagStatusUrl = "http://flagsentry.com/flagstatus.txt";
const char* flagCommandsUrl = "http://flagsentry.com/flagcommand.csv";

// === Global Objects ===
Adafruit_BMP280 bmp280;  // Create Adafruit BMP280 object
//BMP280 bmp280;
String deviceSerial = "";

// Declare global MP3 player objects
AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;


// === Timing intervals (minutes/seconds) ===
const unsigned int sdUpdateIntervalMin     = 1;    // Log to SD every 1 minute
const unsigned int serverUpdateIntervalMin = 2;    // Upload sensor data every 2 minutes
const unsigned int flagCheckIntervalMin    = 1;    // Check flag status every 1 minute
const unsigned int serialDebugIntervalSec  = 10;   // Serial debug every 10 seconds

// === Timing Variables (in millis) ===
unsigned long lastSDUpdate     = 0;
unsigned long lastServerUpdate = 0;
unsigned long lastFlagCheck    = 0;
unsigned long lastSerialDebug  = 0;

// === Cached Sensor Data and HTML content ===
String cachedHTML;
float cachedTempC = 0.0;
float cachedTempF = 0.0;
float cachedPressure = 0.0;
int cachedLightVal = 0;
String cachedLightCond = "";
String cachedTimeStr = "";
float getFreeHeapKB();
float getTotalHeapKB();
float getFreePSRAMKB();
float getTotalPSRAMKB();

// === Function Declarations ===
String getESP32SerialNumber();
void updateSensorData();
void serveMyFlagHTML();
void timedUpdates();
void checkHttpflagStatus();
String getFormattedTime();
void logToSDCard();
void sendSensorData();
void debugSerial();
void handleFlagPositionUpdates();

// --- Persistence Functions ---
FlagPosition readLastFlagPos() {
  if (!SD.exists("/lastflag.txt")) {
    Serial.println("File /lastflag.txt does not exist. Creating it with default value FLAG_DOWN.");
    File f = SD.open("/lastflag.txt", FILE_WRITE);
    if (f) {
      f.print(flagPosToString(FLAG_DOWN));
      f.close();
      return FLAG_DOWN;
    } else {
      Serial.println("Failed to create /lastflag.txt.");
      return FLAG_DOWN;
    }
  } else {
    File f = SD.open("/lastflag.txt", FILE_READ);
    if (f) {
      String posStr = f.readStringUntil('\n');
      f.close();
      posStr.trim();
      Serial.print("Read last flag pos from SD: ");
      Serial.println(posStr);
      return stringToFlagPos(posStr);
    }
  }
  return FLAG_DOWN;
}

void writeLastFlagPos(FlagPosition pos) {
  File f = SD.open("/lastflag.txt", FILE_WRITE);
  if (f) {
    f.print(flagPosToString(pos));
    f.close();
    Serial.print("Stored last flag pos to SD: ");
    Serial.println(flagPosToString(pos));
  } else {
    Serial.println("Failed to open /lastflag.txt for writing.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PHOTOCELL_PIN, INPUT);

  // Initialize I2C for BMP280 sensor.
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);  // 100kHz is usually safe
  //bmp280.setI2CAddress(0x76)
   if (!bmp280.begin(0X76)) {
    Serial.println("❌ BMP280 sensor not found");
    while (1);
  }

  // Initialize SD card.
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("❌ SD card initialization failed");
    while (1); // Stop execution if SD card initialization fails
  }

  //listFiles();  //list the files in the SD card

  // Read last flag position from SD.
  currentFlagStatus = readLastFlagPos();
  // Initialize target and last commanded flag position.
  targetFlagPosition = currentFlagStatus;
  lastCommandedFlagPosition = currentFlagStatus;

  // Generate unique device serial number.
  deviceSerial = getESP32SerialNumber();
  Serial.print("📟 Device Serial: ");
  Serial.println(deviceSerial);

      // Print free memory
     Serial.printf("Heap Memory - Free: %.2f KB, Total: %.2f KB\n", getFreeHeapKB(), getTotalHeapKB());

    // If PSRAM is available, include its stats
    if (ESP.getPsramSize() > 0) {
        Serial.printf("PSRAM - Free: %.2f KB, Total: %.2f KB\n", getFreePSRAMKB(), getTotalPSRAMKB());
    } else {
        Serial.println("PSRAM is not available on this ESP32.");
    }
 // Serial.printf("Free Memory: %.2f KB\n", getFreeMemoryKB());
 // Serial.printf("Total Memory: %.2f KB\n", getTotalMemoryKB());
    //if (ESP.getPsramSize() > 0) {
 // Serial.printf("PSRAM - Free: %.2f KB, Total: %.2f KB\n", getFreePSRAMKB(), getTotalPSRAMKB());
    //} else {
     //   Serial.println("PSRAM is not available on this ESP32.");
    //}

  // Connect to WiFi.
  WiFi.setHostname("myflag");
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected. IP address: ");
  Serial.println(WiFi.localIP());

  // Start mDNS.
  if (!MDNS.begin("myflag")) {
    Serial.println("❌ Error starting mDNS");
  } else {
    Serial.println("🌐 mDNS started: http://myflag.local");
  }
  server.begin();

  // Read HTML file from SD into memory.
  File htmlFile = SD.open("/index.html");
  if (htmlFile) {
    while (htmlFile.available()) {
      cachedHTML += char(htmlFile.read());
    }
    htmlFile.close();
    Serial.println("✅ HTML file cached.");
  } else {
    Serial.println("❌ Failed to cache index.html.");
  }

  // Setup NTP for time synchronization.
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP sync");
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\n✅ Time synchronized");

  // Set timezone (example: New York with DST).
  setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0/2", 1);
  tzset();
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  Serial.printf("UTC:   %s", ctime(&now));  //Output the UTC time
  Serial.printf("Local: %04d-%02d-%02d %02d:%02d:%02d\n",  //Serial out the corrected GMT DST
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);

  Serial.println("Performing initial updates...");
updateSensorData();
  checkHttpflagStatus();
  logToSDCard();
sendSensorData();
  Serial.println("✅ Initial updates completed.");

  // Setup motor control.
  setupMotor();

      // Initialize MP3 player
  setupMP3Player();

}
//*****************************MAIN LOOP*****************************************************************************************
//*******************************************************************************************************************************

void loop() {
    // Centralized serial input handling
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
       // command.trim(); // Remove unnecessary whitespace
        Serial.println("Received command @ Loop Start: [" + command + "]"); // Debugging

        // Dispatch the command
        if (command.startsWith("play ")) {
            Serial.println("Determined it was a play command: [" + command + "]"); // Debugging
            handleMP3Command(command);
        } else if (command == "stop") {
            stopMP3Playback();
        } else {
            handleHTTPCommand(command);

        }
    }
    

    // Other periodic tasks
    updateSensorData();
    handleFlagPositionUpdates();
    handleMP3Playback();
    if (!mp3 || !mp3->isRunning()){
       // Serial.println("🔊 MP3 is running, processing playback...");
       // mp3->loop(); // Process MP3 playback in chunks
            serveMyFlagHTML(server,
                    cachedHTML,
                    cachedTimeStr,
                    deviceSerial,
                    cachedTempC,
                    cachedTempF,
                    cachedPressure,
                    cachedLightVal,
                    cachedLightCond,
                    currentFlagStatus,
                    flagPosToString);
             timedUpdates();
        } else {
            Serial.println("MP3 is playing, deferring SD card logging.");
       }
    
    //delay(10); // Small delay to prevent tight looping
}


//**************************************************************************END OF MAIN LOOP **********************************************************************
void handleMP3Command(const String& command) {
    if (command.startsWith("play ")) {
        String fileName = command.substring(5); // Extract the filename
        fileName.trim();

        if (!fileName.startsWith("/")) {
            fileName = "/" + fileName; // Ensure leading slash
        }

        Serial.println("Attempting to find file: [" + fileName + "]");
        if (SD.exists(fileName.c_str())) {
            Serial.println("✅ File found! Attempting to play...");
        } else {
            Serial.println("❌ File not found on SD card: " + fileName);
            listFiles();  // Show what *is* on SD
        }

        if (!playMP3File(fileName.c_str())) {
            Serial.println("❌ Failed to play the MP3 file.");
        }
    } else {
        Serial.println("Invalid MP3 command.");
    }
}


void handleHTTPCommand(const String& command) {
    if (command == "checkFlagStatus") {
        checkHttpflagStatus();
    } else {
        Serial.println("Unknown HTTP command: [" + command + "]");
    }
} // Added missing closing brace for loop()

// === Helper Functions ===

String getESP32SerialNumber() {
  uint64_t chipid = ESP.getEfuseMac();
  char idString[17];
  sprintf(idString, "%04X%08lX", (uint16_t)(chipid >> 32), (unsigned long)(chipid & 0xFFFFFFFF));
  return String(idString);
}
// Function to calculate free memory
float getFreeHeapKB() {
    return heap_caps_get_free_size(MALLOC_CAP_8BIT) / 1024.0; // Convert bytes to KB
}

float getTotalHeapKB() {
    return heap_caps_get_total_size(MALLOC_CAP_8BIT) / 1024.0; // Convert bytes to KB
}

float getFreePSRAMKB() {
    return ESP.getFreePsram() / 1024.0; // Convert bytes to KB
}

float getTotalPSRAMKB() {
    return ESP.getPsramSize() / 1024.0; // Convert bytes to KB
}

//Sensor Data 
void updateSensorData() {
  cachedTimeStr = getFormattedTime();
  cachedTempC = bmp280.readTemperature();
  cachedTempF = cachedTempC * 1.8 + 32;
  cachedPressure = bmp280.readPressure() / 100.0F;
  cachedLightVal = digitalRead(PHOTOCELL_PIN);     // Read the photocell value
  cachedLightCond = (cachedLightVal < 2048) ? "Light" : "Dark";
}

void timedUpdates() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastSDUpdate >= sdUpdateIntervalMin * 60000UL) {
    lastSDUpdate = currentMillis;
    logToSDCard();
  }
  
  if (currentMillis - lastServerUpdate >= serverUpdateIntervalMin * 60000UL) {
    lastServerUpdate = currentMillis;
    sendSensorData();
  }
  
  if (currentMillis - lastFlagCheck >= flagCheckIntervalMin * 60000UL) {
    lastFlagCheck = currentMillis;
    checkHttpflagStatus();
  }
  
  if (currentMillis - lastSerialDebug >= serialDebugIntervalSec * 1000UL) {
    lastSerialDebug = currentMillis;
    debugSerial();
  }
}

void checkHttpflagStatus() {     //HTTP check flag status on webserver
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(flagStatusUrl);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String httpFlagStatus = http.getString();
      httpFlagStatus.trim();
      Serial.print("📄 Received flag status (HTTP): ");
      Serial.println(httpFlagStatus);
      // Update target flag position from HTTP input.
      updateTargetPositionFromHTTP(httpFlagStatus);
    } else {
      Serial.print("❌ Failed to fetch flag status. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  } else {
    Serial.println("❌ WiFi disconnected. Unable to check flag status.");
  }
}

String getFormattedTime() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

void logToSDCard() {
  File file = SD.open("/log.csv", FILE_APPEND);
  if (file) {
    file.printf("%s,%.2f,%.2f,%.2f,%d,%s,%s\n",
                cachedTimeStr.c_str(), cachedTempC, cachedTempF, cachedPressure,
                cachedLightVal, cachedLightCond.c_str(), flagPosToString(currentFlagStatus));
    file.close();
    Serial.println("✅ Logged to SD card.");
  } else {
    Serial.println("❌ Failed to write to SD card.");
  }
}

void sendSensorData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(dataUploadUrl);
    http.addHeader("Content-Type", "application/json");
    
    String jsonData = "{";
    jsonData += "\"serial\":\"" + deviceSerial + "\",";
    jsonData += "\"tempC\":" + String(cachedTempC, 2) + ",";
    jsonData += "\"tempF\":" + String(cachedTempF, 2) + ",";
    jsonData += "\"pressure\":" + String(cachedPressure, 2) + ",";
    jsonData += "\"ldr\":" + String(cachedLightVal) + ",";
    jsonData += "\"light\":\"" + cachedLightCond + "\",";
    jsonData += "\"flag\":\"" + String(flagPosToString(currentFlagStatus)) + "\",";
    jsonData += "\"time\":\"" + cachedTimeStr + "\"";
    jsonData += "}";
    
    int httpResponseCode = http.POST(jsonData);
    Serial.print("🌐 HTTP Response: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode != 200) {
      String payload = http.getString();
      Serial.println("Response: " + payload);
    }
    
    http.end();
  } else {
    Serial.println("❌ WiFi Disconnected.");
  }
}

void debugSerial() {
  Serial.printf("🕒 Time: %s | 🌡️ Temp: %.2f°C (%.2f°F) | 🌬️ Pressure: %.2f hPa | 💡 Light: %d (%s) | 🚩 Flag: %s\n",
                cachedTimeStr.c_str(), cachedTempC, cachedTempF, cachedPressure,
                cachedLightVal, cachedLightCond.c_str(), flagPosToString(currentFlagStatus));
}
void handleFlagPositionUpdates() {
    static unsigned long lastFlagUpdate = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastFlagUpdate >= 1000) { // Update every second
        lastFlagUpdate = currentMillis;

        // Process manual inputs
        updateTargetPositionFromSerial();

        // Process photocell input for AUTO mode
        if (targetFlagPosition == FLAG_AUTO) {
            updateTargetPositionFromPhotocell(cachedLightVal);
        }

        // Apply the target if it has changed
        applyTargetPosition();

        // Update motor movement
        updateMotorMovement();

        // If the target position differs from the current, command the motor
        if (targetFlagPosition != currentFlagStatus && targetFlagPosition != FLAG_AUTO) {
            Serial.printf("Moving flag from %s to %s\n",
                          flagPosToString(currentFlagStatus),
                          flagPosToString(targetFlagPosition));
            moveToPosition(String(flagPosToString(targetFlagPosition)));
            currentFlagStatus = targetFlagPosition;
            writeLastFlagPos(currentFlagStatus);
        }
    }
}
void listFiles() {
    Serial.print("Reading Files");
    File root = SD.open("/");
    while (true) {
        File entry = root.openNextFile();
        if (!entry) break; // No more files
        Serial.print("File: ");
        Serial.println(entry.name());
        entry.close();
    }
}
