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
    NFCReader reader("/dev/i2c-2", 0x24);

     std::this_thread::sleep_for(std::chrono::seconds(3));

    bool inEnglish = false;
    bool inFrench = false;

    int counter = 0;
    // Main loop
    while(!shutdownManager.isShutdown()) {

        std::string uid = reader.waitForCardAndReadUID();
        std::cout << "UID = " << uid << std::endl;

        // check if entry is 01:00:e1:44
        if (uid == "01:00:e1:44" && inEnglish == false) {
            std::cout << "UID is ENGLISH" << std::endl;
            audioMixer.queueSound(ENGLISH);
            inEnglish = true;
            counter = 0;
        } else if (uid == "01:00:e1:04" && inFrench == false){
            std::cout << "UID is FRENCH" << std::endl;
            audioMixer.queueSound(FRENCH);
            inFrench = true;
            counter = 0;
        } 

        if (counter == 2) {
            inEnglish = false;
            inFrench = false;
            counter = 0;
        }

        counter++;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    shutdownManager.waitForShutdown();
}