#include "textToSpeech.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

TextToSpeech::TextToSpeech() {
}

TextToSpeech::~TextToSpeech() {
}

void TextToSpeech::translateToWave(char* message , char* filename) {
    //TODO: Use strings, use fstream in runCommand()
    char buffer[256];
    snprintf(buffer, 256, "espeak '%s' -w %s", message, filename);
    runCommand(buffer);
    // Allow 1 second for espeak processing, this should be more than enough
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void TextToSpeech::runCommand(char* command) {


    FILE *pipe = popen(command, "r");
    char buffer[1024];

    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
        break;
    }

    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        printf("Unable to execute command: %s, exit code: %d\n", command, exitCode);
        exit(1);
    }
}