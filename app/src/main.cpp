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
    NFCReader reader("/dev/i2c-2", 0x24);

    while(!shutdownManager.isShutdown()) {
        std::string uid = reader.waitForCardAndReadUID();
        std::cout << "UID = " << uid << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    shutdownManager.waitForShutdown();
}