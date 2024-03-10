// Text-to-speech Module
// This module has ONE sole responsibility, it can take a message and a filename
// and use 'espeak' to translate this to a MONO 22050 Hz audio sample in .wav format.
// translateToWave should get used by the UDP module, detecting updates from the web server.

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