/*MP3player.cpp by Tim Nash not for distribution */
#include "MP3player.h"

// Define global MP3 player objects
AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;

// Define the playback timer
unsigned long playbackStartTime = 0;

void setupMP3Player() {
    // Set up I2S output on I2S1 (secondary peripheral)
    out = new AudioOutputI2S();  
    out->SetOutputModeMono(true);   // Mono output = fewer channels = less peripheral use
    out->SetPinout(BCLK_PIN, LRCLK_PIN, DOUT_PIN);
    out->SetGain(0.4);              // Set volume (0.0 to 1.0)
    // mp3 = new AudioGeneratorMP3(); // (left commented as you had it)
    file = nullptr;
}

bool isMP3Playing() {
    if (!mp3) {
        Serial.println("MP3 object is null.");
        return false;
    }
    return mp3->isRunning();
}

bool playMP3File(const char *fileName) {
    // stopMP3Playback(); // Stop any existing playback
    Serial.println("Jumped to playMP3File \n");

        // Stop any existing playback first
    stopMP3Playback();

    // Check if the file exists
    if (!SD.exists(fileName)) {
        Serial.printf("playMP3File Failed to find: %s\n", fileName);
        return false; // Return false if the file does not exist
    }

    // Reinitialize necessary components
    file = new AudioFileSourceSD(fileName);
    mp3 = new AudioGeneratorMP3();
    // out = new AudioOutputI2S(); // (left commented as you had it)

    // Begin playback
    if (!mp3->begin(file, out)) { // Check if mp3->begin() fails
        Serial.println("❌ mp3->begin() failed.");
        return false; // Return false if playback initialization fails
    }

    // Start the 2-minute timer
    playbackStartTime = millis(); // <-- IMPORTANT: start timing when playback begins

    // Log successful playback
    Serial.printf("🎵 Playing '%s'...\n", fileName);
    return true; // Return true to indicate playback successfully started
}

void handleMP3Playback() {
    if (mp3) {
        bool stillRunning = mp3->loop(); // <-- capture the result of loop()

      //  Serial.println("handleMP3Playback: loop returned " + String(stillRunning) + ", isRunning = " + String(mp3->isRunning()));

        if (!stillRunning) {
            Serial.println("MP3 loop() ended. Stopping playback.");
            stopMP3Playback();
        }
    }
}


void stopMP3Playback() {
    if (mp3) {
        if (mp3->isRunning()) {
            mp3->stop();
        }
        delete mp3;
        mp3 = nullptr;
    }

    if (file) {
        delete file;
        file = nullptr;
    }

    // We do not delete 'out' here — we keep the I2S peripheral alive for future plays

    playbackStartTime = 0; // Reset the timer
    Serial.println("MP3 playback stopped.");
}
