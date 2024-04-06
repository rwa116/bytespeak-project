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

    for(enum Language language : languageManager->getDefaultLanguages()) {
        std::string filename = languageManager->getWavFilename(language);
        std::ifstream file(filename);
        if(!file.good()) {
            std::ofstream newFile(filename);
            // defaultMessage if ENGLISH, otherwise translate
            std::string message = language == ENGLISH ? defaultMessage
                : translator->translateToLanguage(defaultMessage, language);
            translateToWave(message, language, filename);
        }
    }
}

TextToSpeech::~TextToSpeech() {
}

void TextToSpeech::translateToWave(std::string message, enum Language language, std::string filename, enum Gender gender) {
    std::string languageCode;
    std::string genderCode;
    switch(language) {
        case FRENCH:
            languageCode = "fr_FR";
            switch(gender) {
                default:
                    genderCode = "gilles-low";
                    break;
            }
            break;
        case GERMAN:
            languageCode = "de_DE";
            switch(gender) {
                default:
                    genderCode = "eva_k-x_low";
                    break;
            }
            break;
        case ENGLISH:
            languageCode = "en_US";
            switch(gender) {
                default:
                    genderCode = "amy-low";
                    break;
            }
            break;
        case SPANISH:
            languageCode = "es_ES";
            switch(gender) {
                default:
                    genderCode = "carlfm-x_low";
                    break;
            }
            break;
        case CHINESE:
            languageCode = "zh_CN";
            switch(gender) {
                default:
                    genderCode = "huayan-x_low";
                    break;
            }
            break;
        default:
            return;
    } 
    // Create new file
    std::string model;
    model = "/mnt/remote/myApps/bytespeak-voices/" + languageCode + "-" + genderCode + ".onnx";
    std::stringstream command;
    command << "echo \"" << message << "\" | ./piper/piper --model " << model << " --output_file " << filename;

    runCommand(command.str());

    // Allow 2 seconds after processing
    std::this_thread::sleep_for(std::chrono::seconds(2));
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