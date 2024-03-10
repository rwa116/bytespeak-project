// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "shutdown.hpp"
#include "textToSpeech.hpp"
#include "network.hpp"

#include "hal/audioMixer.hpp"
#include "hal/nfc.hpp"

int main() {
    AudioMixer audioMixer;
    TextToSpeech textToSpeech;
    ShutdownManager shutdownManager;
    Network network(&shutdownManager, &textToSpeech, &audioMixer);

    while(!shutdownManager.isShutdown()) {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        audioMixer.queueSound(ENGLISH);
    }

    shutdownManager.waitForShutdown();
}