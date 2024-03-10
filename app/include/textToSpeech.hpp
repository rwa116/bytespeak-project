// Text-to-speech Module

#ifndef _TEXT_TO_SPEECH_HPP_
#define _TEXT_TO_SPEECH_HPP_

#include <string>

class TextToSpeech {
public:
    TextToSpeech();
    ~TextToSpeech();
    void translateToWave(std::string message , std::string filename);
private:
    void runCommand(std::string command);
};

#endif