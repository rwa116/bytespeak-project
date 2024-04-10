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
#include "stateReader.hpp"
#include "hal/languageManager.hpp"
#include "hal/potController.hpp"
#include "hal/display.hpp"

#include "hal/audioMixer.hpp"
#include "hal/nfc.hpp"
#include "translator.hpp"

int main() {
    LanguageManager languageManager;
    Translator translator;
    TextToSpeech textToSpeech(&languageManager, &translator);
    AudioMixer audioMixer(&languageManager);
    ShutdownManager shutdownManager;
    StateReader stateReader(&audioMixer);
    Network network(&shutdownManager, &languageManager, &textToSpeech, &translator, &audioMixer);
    //NFCReader reader("/dev/i2c-2", 0x24);

    // Main loop
    while(!shutdownManager.isShutdown()) {
        audioMixer.queueSound(ENGLISH);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        audioMixer.queueSound(FRENCH);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        audioMixer.queueSound(GERMAN);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        audioMixer.queueSound(SPANISH);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        audioMixer.queueSound(CHINESE);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // std::string uid = reader.waitForCardAndReadUID();
        // std::cout << "UID = " << uid << std::endl;
        // // std::this_thread::sleep_for(std::chrono::seconds(1));
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        audioMixer.queueSound(CUSTOM_1);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(CUSTOM_2);
    }

    shutdownManager.waitForShutdown();
}