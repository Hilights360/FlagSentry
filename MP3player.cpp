#include "MP3player.h"
//#include <AudioGeneratorMP3.h>
//#include <AudioFileSourceSD.h>
//#include <AudioOutputI2S.h>
//#include <Audio.h>



void setupMP3Player() {

    // Set up I2S output on I2S1 (secondary peripheral)
    out = new AudioOutputI2S();  // 
    out->SetOutputModeMono(true);   // Mono output = fewer channels = less peripheral use
    out->SetPinout(BCLK_PIN, LRCLK_PIN, DOUT_PIN);
    out->SetGain(0.4);            // Set volume (0.0 to 1.0)
}

bool isMP3Playing() {
    if (!mp3) {
        Serial.println("MP3 object is null.");
        return false;
    }
    return mp3->isRunning();
}

bool playMP3File(const char *fileName) {
    stopMP3Playback(); // Stop any existing playback
    Serial.println("Jumped to playMP3File \n");
    

    // Check if the file exists
    if (!SD.exists(fileName)) {
        Serial.printf("Failed to find: %s\n", fileName);
        return false; // Return false if the file does not exist
    }
    // Set up MP3 file source
    file = new AudioFileSourceSD(fileName);

    // Set up MP3 decoder
    mp3 = new AudioGeneratorMP3();

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
    if (mp3) { // Ensure mp3 is not null
        Serial.print("isRunning value: ");
        Serial.println(mp3->isRunning()); // Debugging output for isRunning
    } else {
        Serial.println("mp3 object is null.");
    }

    if (mp3 && mp3->isRunning()) {
        mp3->loop(); // Continue MP3 playback
    } else {
        stopMP3Playback(); // Stop playback if MP3 is not running
    }
}


void stopMP3Playback() {
    if (mp3 && mp3->isRunning()) {
        Serial.println("Stopping MP3 playback...");
        mp3->stop();
        delete mp3;
        mp3 = nullptr;
        Serial.println("MP3 playback stopped successfully.");
    } 
}