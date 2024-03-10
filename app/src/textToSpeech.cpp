#include "textToSpeech.hpp"

#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

TextToSpeech::TextToSpeech() {
}

TextToSpeech::~TextToSpeech() {
}

void TextToSpeech::translateToWave(std::string message , std::string filename) {
    std::stringstream command;
    command << "espeak '" << message << "' -w " << filename;

    runCommand(command.str());
    // Allow 1 second for espeak processing, this should be more than enough
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