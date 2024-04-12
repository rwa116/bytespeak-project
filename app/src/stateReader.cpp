#include "stateReader.hpp" 
#include "hal/audioMixer.hpp" 

#include <iostream>

StateReader::StateReader(AudioMixer *audioMixerInstance, LanguageManager *languageManagerInstance, LEDPanel *ledPanelInstance) {
    isRunning = true;
    audioMixer = audioMixerInstance;
    languageManager = languageManagerInstance;
    ledPanel = ledPanelInstance;
    i2cFileDesc = display.getDesc();

    stateReaderThreadId = std::thread([this]() {
        this->stateReaderThread(nullptr);
    });
}

StateReader::~StateReader() {
    isRunning = false;
    stateReaderThreadId.join();
}

#define POT_MAX_VOLUME (4095.0 - (4095.0 / 100.0))
#define POT_VOL_INCREMENT (POT_MAX_VOLUME / 100.0)
void *StateReader::stateReaderThread(void *arg) {
    (void)arg;
    long long lastTime = getCurrentTimeMs();
    while(isRunning) {
        int potValue = potController.getReading();

        int newVolume = (int) ((double)potValue / POT_VOL_INCREMENT);
        audioMixer->setVolume(newVolume);
        display.writeNumber(i2cFileDesc, newVolume / 10);

        long long currentTime = getCurrentTimeMs();
        if(currentTime > lastTime + 300 && pruDriver.isRightPressed()) {
            lastTime = currentTime;
            // std::cout << "Right Pressed" << std::endl;
            languageManager->cycleLanguage();
            enum Language currentLanguage = languageManager->getCurrentLanguage();
            audioMixer->queueSound(currentLanguage);
            ledPanel->displayFlag(currentLanguage);
        }
        if(currentTime > lastTime + 500 && pruDriver.isDownPressed()) {
            lastTime = currentTime;
            // std::cout << "Down Pressed" << std::endl;
            enum Language currentLanguage = languageManager->getCurrentLanguage();
            audioMixer->queueSound(currentLanguage);
            ledPanel->displayFlag(currentLanguage);
        }

        sleepForMs(100);

    }
    return nullptr;
}


void StateReader::sleepForMs(long long delayInMs) {
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

long long StateReader::getCurrentTimeMs() {
    auto currentTime = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime).time_since_epoch().count();
}