#include "textToSpeech.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>

TextToSpeech::TextToSpeech(LanguageManager *languageManagerReference, Translator *translatorReference) {
    languageManager = languageManagerReference;
    translator = translatorReference;

    // Initial message setup
    std::string defaultMessage = "This is a default message";

    std::string englishFilename = languageManager->getWavFilename(ENGLISH);
    std::ifstream englishFile(englishFilename);
    if(!englishFile.good()) {
        std::ofstream newFile(englishFilename);
        std::cout <<"reach" << std::endl;
        translateToWave(defaultMessage, ENGLISH, englishFilename);
    }

    std::string frenchFilename = languageManager->getWavFilename(FRENCH);
    std::ifstream frenchFile(frenchFilename);
    if(!frenchFile.good()) {
        std::ofstream newFile(frenchFilename);
        std::string frenchMessage = translator->translateToLanguage(defaultMessage, FRENCH);
        translateToWave(frenchMessage, FRENCH, frenchFilename);
    }

    std::string germanFileName = languageManager->getWavFilename(GERMAN);
    std::ifstream germanFile(germanFileName);
    if(!germanFile.good()) {
        std::ofstream newFile(germanFileName);
        std::string germanMessage = translator->translateToLanguage(defaultMessage, GERMAN);
        translateToWave(germanMessage, GERMAN, germanFileName);
    }
}

TextToSpeech::~TextToSpeech() {
}

void TextToSpeech::translateToWave(std::string message, enum Language language, std::string filename) {
    std::string languageCode;
    switch(language) {
        case FRENCH:
            languageCode = "fr";
            break;
        case GERMAN:
            languageCode = "de";
            break;
        default:
            languageCode = "en";
            break;
    } 
    std::stringstream command;
    command << "espeak '" << message << "' -v" << languageCode << " -w " << filename;

    runCommand(command.str());
    // Allow 2 second for espeak processing, this should be more than enough
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void TextToSpeech::runCommand(std::string command) {

    FILE *pipe = popen(command.c_str(), "r");
    char buffer[1024];

    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
        break;
    }

    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        std::cerr << "Unable to execute command: " << command 
            << ", exit code: " << exitCode << std::endl;
        exit(1);
    }
}