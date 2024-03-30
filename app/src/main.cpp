// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>

#include "shutdown.hpp"
#include "textToSpeech.hpp"
#include "network.hpp"
#include "translator.hpp"
#include "hal/languageManager.hpp"

#include "hal/audioMixer.hpp"
#include "hal/nfc.hpp"
#include "translator.hpp"

int main() {
    LanguageManager languageManager;
    Translator translator;
    TextToSpeech textToSpeech(&languageManager, &translator);
    AudioMixer audioMixer(&languageManager);
    ShutdownManager shutdownManager;
    Network network(&shutdownManager, &languageManager, &textToSpeech, &translator, &audioMixer);
    //NFCReader reader("/dev/i2c-2", 0x24);

    // Main loop
    while(!shutdownManager.isShutdown()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        audioMixer.queueSound(ENGLISH);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        audioMixer.queueSound(FRENCH);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        audioMixer.queueSound(GERMAN);
        // std::string uid = reader.waitForCardAndReadUID();
        // std::cout << "UID = " << uid << std::endl;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::seconds(3));
        audioMixer.queueSound(CUSTOM_1);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        audioMixer.queueSound(CUSTOM_2);
    }

    shutdownManager.waitForShutdown();
}