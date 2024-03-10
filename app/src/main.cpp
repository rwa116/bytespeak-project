// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "shutdown.hpp"

#include "hal/audioMixer.hpp"
#include "hal/nfc.hpp"

int main() {
    ShutdownManager shutdownManager;
    AudioMixer audioMixer;

    char filename[] = "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav";
    static wavedata_t baseDrumData;
    audioMixer.readWaveFileIntoMemory(filename, &baseDrumData);

    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        audioMixer.queueSound(&baseDrumData);
    }

    shutdownManager.waitForShutdown();
}