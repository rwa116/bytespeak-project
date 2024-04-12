// State Reader Module
// Responsible for coordinating communication between POT and 14-segment display

#ifndef _STATE_READER_HPP_
#define _STATE_READER_HPP_

#include "hal/display.hpp"
#include "hal/potController.hpp"
#include "hal/audioMixer.hpp"
#include "hal/pruDriver.hpp"
#include "hal/ledPanel.hpp"
#include "hal/buzzer.hpp"

class StateReader {
    public:
        StateReader(AudioMixer *audioMixerInstance, LanguageManager *languageManagerInstance, LEDPanel *ledPanelInstance);
        ~StateReader();
    private:
        std::thread stateReaderThreadId;
        Display display;
        PotController potController;
        PruDriver pruDriver;
        AudioMixer *audioMixer;
        LanguageManager *languageManager;
        LEDPanel *ledPanel;
        

        int i2cFileDesc;        

        bool isRunning = false;
        void *stateReaderThread(void *arg);
        void sleepForMs(long long delayInMs);
        long long getCurrentTimeMs();
};


#endif