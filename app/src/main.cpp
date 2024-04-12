#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sys/resource.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <map>

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
#include "hal/ledPanel.hpp"

int main() {
    const rlim_t memory_limit = 500 * 1024 * 1024; // 200 MiB in bytes

    struct rlimit rlim;
    rlim.rlim_cur = memory_limit;
    rlim.rlim_max = memory_limit;

    if (setrlimit(RLIMIT_AS, &rlim) != 0) {
        std::cerr << "Failed to set memory limit: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Memory limit set to " << memory_limit << " bytes" << std::endl;

    std::map<std::string, enum Language> languageMap;
    languageMap["44:20:07:04"] = ENGLISH;
    languageMap["04:08:04:a2"] = FRENCH;
    languageMap["04:09:04:c2"] = GERMAN;

    LEDPanel ledDisplay;
    LanguageManager languageManager;
    Translator translator;
    TextToSpeech textToSpeech(&languageManager, &translator);
    AudioMixer audioMixer(&languageManager);
    ShutdownManager shutdownManager;
    StateReader stateReader(&audioMixer, &languageManager, &ledDisplay);
    Network network(&shutdownManager, &languageManager, &textToSpeech, &translator, &audioMixer);
    NFCReader reader("/dev/i2c-2", 0x24);

    // Main loop
    while(!shutdownManager.isShutdown()) {

        // Test Language Dictation
        // audioMixer.queueSound(ENGLISH);
        // std::cout << "displaying ENGLISH\n";
        // ledDisplay.displayFlag(ENGLISH);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(FRENCH);
        // std::cout << "displaying FRENCH\n";
        // ledDisplay.displayFlag(FRENCH);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(GERMAN);
        // std::cout << "displaying GERMAN\n";
        // ledDisplay.displayFlag(GERMAN);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        
        // audioMixer.queueSound(SPANISH);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(CHINESE);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(CUSTOM_1);
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // audioMixer.queueSound(CUSTOM_2);

        // Test NFC Reader
        std::string uid = reader.waitForCardAndReadUID();
        std::cout << "UID = " << uid << std::endl;
        languageManager.setLanguage(languageMap[uid]);
        audioMixer.queueSound(languageMap[uid]);
        ledDisplay.displayFlag(languageMap[uid]);
        // std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
    }

    shutdownManager.waitForShutdown();
}