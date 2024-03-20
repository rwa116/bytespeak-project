#include <pthread.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>
#include <sstream>
#include <string>

#include "network.hpp"

Network::Network() {
    isRunning = true;

    networkThreadId = std::thread([this]() {
        this->networkThread(nullptr);
    });
}

Network::Network(ShutdownManager *shutdownInstance, LanguageManager *languageManagerInstance, TextToSpeech *textInstance, 
            Translator *translatorInstance, AudioMixer *audioInstance) {
    isRunning = true;
    shutdownManager = shutdownInstance;
    languageManager = languageManagerInstance;
    textToSpeech = textInstance;
    translator = translatorInstance;
    audioMixer = audioInstance;

    networkThreadId = std::thread([this]() {
        this->networkThread(nullptr);
    });
}

Network::~Network() {
    isRunning = false;
    networkThreadId.join();
}

#define MAX_LEN 1500
#define PORT 12345
void *Network::networkThread(void *arg) {
    (void)arg;
    enum Command lastCommand = UNKNOWN;
    // Socket initialization
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketDescriptor == -1) {
        perror("Failed to establish server socket");
        exit(1);
    }
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

    // Message receive and reply loop
    while(!shutdownManager->isShutdown()) {
        struct sockaddr_in sinRemote;
        unsigned int sinLen = sizeof(sinRemote);
        char messageRx[MAX_LEN];

        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, 
            (struct sockaddr*) &sinRemote, &sinLen);

        messageRx[bytesRx] = 0;
        enum Command sentCommand = checkCommand(messageRx);
        lastCommand = sentCommand;
        sendReply(lastCommand, messageRx, socketDescriptor, &sinRemote);
    }

    close(socketDescriptor);

    return NULL;
}

enum Command Network::checkCommand(char* input) {
    std::istringstream istream(input);
    std::string token;
    istream >> token;
    if(token.find("espeak") != std::string::npos) {
        return ESPEAK;
    }
    else if(token.find("terminate") != std::string::npos) {
        return TERMINATE;
    }
    else {
        return UNKNOWN;
    }
}

#define ENGLISH_MESSAGE_FILENAME "beatbox-wav-files/message.wav"
void Network::sendReply(enum Command command, char *input, int socketDescriptor, struct sockaddr_in *sinRemote) {
    char messageTx[MAX_LEN];
    unsigned int sinLen;

    switch(command) {
        case ESPEAK:
        {
            char message[256];
            char* spacePosition = strchr(input, ' ');
            if(spacePosition != nullptr) {
                strncpy(message, spacePosition + 1, 255);
                message[255] = '\0';

                std::string englishFilename = languageManager->getWavFilename(ENGLISH);
                textToSpeech->translateToWave(message, ENGLISH, englishFilename);
                audioMixer->readWaveFileIntoMemory(englishFilename, ENGLISH);

                std::string frenchFilename = languageManager->getWavFilename(FRENCH);
                std::string frenchMessage = translator->translateToLanguage(message, FRENCH);
                textToSpeech->translateToWave(frenchMessage, FRENCH, frenchFilename);
                audioMixer->readWaveFileIntoMemory(frenchFilename, FRENCH);

                std::string germanFileName = languageManager->getWavFilename(GERMAN);
                std::string germanMessage = translator->translateToLanguage(message, GERMAN);
                textToSpeech->translateToWave(germanMessage, GERMAN, germanFileName);
                audioMixer->readWaveFileIntoMemory(germanFileName, GERMAN);
            }
            snprintf(messageTx, MAX_LEN, "Set message to %s\n", message);
            break;
        }
        case TERMINATE:
            shutdownManager->signalShutdown();
            snprintf(messageTx, MAX_LEN, "Terminating\n");
            break;
        case UNKNOWN:
            snprintf(messageTx, MAX_LEN, "Unknown command.\n");
            break;
        default:
            perror("Invalid command");
            exit(1);
    }
    
    sinLen = sizeof(*sinRemote);
    sendto(socketDescriptor, messageTx, strlen(messageTx), 0, 
        (struct sockaddr*) sinRemote, sinLen);
}