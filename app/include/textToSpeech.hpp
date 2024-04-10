// Text-to-speech Module
// This module has ONE sole responsibility, it can take a message and a filename
// and use 'espeak' to translate this to a MONO 22050 Hz audio sample in .wav format.
// translateToWave should get used by the UDP module, detecting updates from the web server.

#ifndef _TEXT_TO_SPEECH_HPP_
#define _TEXT_TO_SPEECH_HPP_

#include <string>
#include <unordered_map>

#include <hal/audioMixer.hpp>
#include <hal/languageManager.hpp>
#include <translator.hpp>

enum Gender {
    MALE,
    FEMALE,
    DEFAULT,
};

class TextToSpeech {
public:
    TextToSpeech(LanguageManager *languageManagerReference, Translator *translatorReference);
    ~TextToSpeech();
    void translateToWave(std::string message, enum Language language, std::string filename, enum Gender gender = DEFAULT);
    enum Gender getCurrentGender(enum Language language);
private:
    LanguageManager *languageManager;
    Translator *translator;
    std::unordered_map<enum Language, enum Gender> languageGenderMap;
    void runCommand(std::string command);
};

#endif