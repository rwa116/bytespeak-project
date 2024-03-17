#include "translator.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#define TRANSLATOR "./trans "
Translator::Translator() {
}

Translator::~Translator() {
}

std::string Translator::translateToLanguage(std::string message , enum Language language) {
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
    std::string command = std::string(TRANSLATOR) + "-b en:" + languageCode + " " + message;
    return runCommand(command);
}

std::string Translator::runCommand(std::string command) {
    FILE* pipe = popen(command.c_str(), "r");
    char buffer[1024];
    std::string output;

    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;

        output += buffer;
    }

    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        std::cerr << "Unable to execute command: " << command
                  << ", exit code: " << exitCode << std::endl;
        exit(1);
    }

    std::cout << "Translated string = " << output << std::endl;
    return output;
}