// Translator Module

#ifndef _TRANSLATOR_HPP_
#define _TRANSLATOR_HPP_

#include <string>
#include <hal/audioMixer.hpp>

class Translator {
public:
    Translator();
    ~Translator();
    std::string translateToLanguage(std::string message , enum Language language);
    void updateCurrentMessageTextFile(std::string message);
    std::string getCurrentMessage();
private:
    std::string runCommand(std::string command);
    std::string currentMessage;
};

#endif