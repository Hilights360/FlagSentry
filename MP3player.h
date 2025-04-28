/*Play audio
Tim Nash 2025**/
#pragma once

#include <AudioGeneratorMP3.h>
#include <AudioFileSourceSD.h>
#include <AudioOutputI2S.h>

class MP3player {
public:
    MP3player();
    void begin();
    void play(const char* filename);
    void stop();
    bool isRunning();
};

// Declare MP3-related global variables
extern AudioGeneratorMP3 *mp3;
extern AudioFileSourceSD *file;
extern AudioOutputI2S *out;

// Declare the playback timer as extern
extern unsigned long playbackStartTime;

#define DOUT_PIN 4  // Data Out (DIN on MAX98357)
#define BCLK_PIN 14  // Bit Clock (BCLK on MAX98357)
#define LRCLK_PIN 13 // Left/Right Clock (LRCLK on MAX98357)


// Function Declarations
void setupMP3Player();
void stopMP3Playback();
bool isMP3Playing();
void handleMP3Playback();
bool playMP3File(const char *fileName);
