// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "shutdown.hpp"
#include "textToSpeech.hpp"

#include "hal/audioMixer.hpp"
#include "hal/nfc.hpp"

int main() {
    ShutdownManager shutdownManager;
    AudioMixer audioMixer;
    TextToSpeech textToSpeech;

    char message[] = "This is the first text to speech test.";
    char filename[] = "beatbox-wav-files/message.wav";
    textToSpeech.translateToWave(message, filename);

    static wavedata_t messageData;
    audioMixer.readWaveFileIntoMemory(filename, &messageData);

    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        audioMixer.queueSound(&messageData);
    }

    shutdownManager.waitForShutdown();
}