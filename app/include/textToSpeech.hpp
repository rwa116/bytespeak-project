// Text-to-speech Module

#ifndef _TEXT_TO_SPEECH_HPP_
#define _TEXT_TO_SPEECH_HPP_

class TextToSpeech {
public:
    TextToSpeech();
    ~TextToSpeech();
    void translateToWave(char *message ,char *filename);
private:
    void runCommand(char* command);
};

#endif