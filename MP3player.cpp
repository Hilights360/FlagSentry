#include "MP3player.h"

// Define global MP3 player objects
AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;

// Define the playback timer
unsigned long playbackStartTime = 0;

void setupMP3Player() {

    // Set up I2S output on I2S1 (secondary peripheral)
    out = new AudioOutputI2S();  // 
    out->SetOutputModeMono(true);   // Mono output = fewer channels = less peripheral use
    out->SetPinout(BCLK_PIN, LRCLK_PIN, DOUT_PIN);
    out->SetGain(0.4);            // Set volume (0.0 to 1.0)
    //mp3 = new AudioGeneratorMP3();
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
    //stopMP3Playback(); // Stop any existing playback
    Serial.println("Jumped to playMP3File \n");
    

    // Check if the file exists
    if (!SD.exists(fileName)) {
        Serial.printf("playMP3File Failed to find: %s\n", fileName);
        return false; // Return false if the file does not exist
    }
    // Reinitialize necessary components
    // Set up MP3 file source
    file = new AudioFileSourceSD(fileName);
    // Set up MP3 decoder
    mp3 = new AudioGeneratorMP3();
    //out = new AudioOutputI2S();

    // Begin playback
    if (!mp3->begin(file, out)) { // Check if mp3->begin() fails
        Serial.println("❌ mp3->begin() failed.");
        return false; // Return false if playback initialization fails
    }

    // Log successful playback
    Serial.printf("🎵 Playing '%s'...\n", fileName);
    return true; // Return true to indicate playback successfully started
}


void handleMP3Playback() {
    if (mp3 && mp3->isRunning()) {
        if (millis() - playbackStartTime >= 120000) { // 2-minute timer
            Serial.println("2-minute timer reached. Stopping playback.");
            stopMP3Playback();
        } else {
            mp3->loop(); // Continue playback
        }
    } else {
        stopMP3Playback();
    }
}



void stopMP3Playback() {
    if (mp3 && mp3->isRunning()) {
        mp3->stop();
    }

    if (file) delete file;
    if (out) delete out;

    mp3 = nullptr;
    file = nullptr;
    out = nullptr;

    playbackStartTime = 0; // Reset the timer
    Serial.println("MP3 playback stopped.");
}    