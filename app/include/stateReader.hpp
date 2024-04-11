// State Reader Module
// Responsible for coordinating communication between POT and 14-segment display

#ifndef _STATE_READER_HPP_
#define _STATE_READER_HPP_

#include "hal/display.hpp"
// #include "hal/segDisplay.hpp"
#include "hal/potController.hpp"
#include "hal/audioMixer.hpp"

class StateReader {
    public:
        StateReader(AudioMixer *audioMixerInstance);
        ~StateReader();
    private:
        std::thread stateReaderThreadId;
        Display display;
        PotController potController;
        AudioMixer *audioMixer;

        int i2cFileDesc;        

        bool isRunning = false;
        void *stateReaderThread(void *arg);
        void sleepForMs(long long delayInMs);
};


#endif