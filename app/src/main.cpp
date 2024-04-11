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
    const rlim_t memory_limit = 500 * 1024 * 1024; // 200 MiB in bytes

    struct rlimit rlim;
    rlim.rlim_cur = memory_limit;
    rlim.rlim_max = memory_limit;

    if (setrlimit(RLIMIT_AS, &rlim) != 0) {
        std::cerr << "Failed to set memory limit: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Memory limit set to " << memory_limit << " bytes" << std::endl;

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