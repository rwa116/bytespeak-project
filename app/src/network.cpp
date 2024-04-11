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
#include <fstream>

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

#define MAX_LEN 65200
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
        sendReply(lastCommand, messageRx, bytesRx, socketDescriptor, &sinRemote);
    }

    close(socketDescriptor);

    return NULL;
}

enum Command Network::checkCommand(char* input) {
    // If all packets have been sent for our custom language wav, return to normal mode
    if(numPacketsLeft <= 0) {
        mode = NORMAL;
    }

    if(mode == NORMAL) {
        std::istringstream istream(input);
        std::string token;
        istream >> token;
        if(token.find("espeak") != std::string::npos) {
            return ESPEAK;
        }
        if(token.find("setVoice") != std::string::npos) {
            return SET_VOICE;
        }
        if(token.find("getInfo") != std::string::npos) {
            return GET_INFO;
        }
        if(token.find("cl1") != std::string::npos) {
            return CL1;
        }
        if(token.find("cl2") != std::string::npos) {
            return CL2;
        }
        else if(token.find("terminate") != std::string::npos) {
            return TERMINATE;
        }
        else {
            return UNKNOWN;
        }
    }

    else {
        numPacketsLeft--;
        return SENDING_DATA;
    }
}

#define ENGLISH_MESSAGE_FILENAME "beatbox-wav-files/message.wav"
void Network::sendReply(enum Command command, char *input, int length, int socketDescriptor, struct sockaddr_in *sinRemote) {
    char messageTx[MAX_LEN];
    unsigned int sinLen;

    switch(command) {
        case ESPEAK:
        {
            char message[512];
            char* spacePosition = strchr(input, ' ');
            if(spacePosition != nullptr) {
                strncpy(message, spacePosition + 1, 511);
                message[511] = '\0';
                translator->updateCurrentMessageTextFile(message);

                for(enum Language currentLanguage : languageManager->getDefaultLanguages()) {
                    std::string filename = languageManager->getWavFilename(currentLanguage);

                    // defaultMessage if ENGLISH, otherwise translate
                    std::string newMessage = currentLanguage == ENGLISH ? message
                        : translator->translateToLanguage(message, currentLanguage);

                    pthread_mutex_lock(&audioMixer->fileAccessMutex);
                    textToSpeech->translateToWave(newMessage, currentLanguage, filename);
                    pthread_mutex_unlock(&audioMixer->fileAccessMutex);

                    audioMixer->readWaveFileIntoMemory(filename, currentLanguage);
                }
            }
            snprintf(messageTx, MAX_LEN, "currentMessage;%s", message);
            break;
        }
        case GET_INFO:
            snprintf(messageTx, MAX_LEN, "currentMessage;%s", translator->getCurrentMessage().c_str());
            break;
        case SET_VOICE:
        {
            std::istringstream istream(input);
            std::string token;
            istream >> token;
            istream >> token;

            enum Language language;
            enum Gender gender;
            std::cout << "Token: " << token << "\n";

            if(token.find("en-US") != std::string::npos) {
                std::cout << "English\n";
                language = ENGLISH;
            }
            else if(token.find("fr-FR") != std::string::npos) {
                language = FRENCH;
            }
            else if(token.find("de-DE") != std::string::npos) {
                language = GERMAN;
            }
            else if(token.find("es-ES") != std::string::npos) {
                std::cout << "Spanish\n";
                language = SPANISH;
            }
            else if(token.find("zh-CN") != std::string::npos) {
                language = CHINESE;
            }
            else {
                snprintf(messageTx, MAX_LEN, "Unknown language\n");
                break;
            }

            istream >> token;
            std::string message = translator->getCurrentMessage();

            if(token.find("m") != std::string::npos) {
                gender = MALE;
            }
            else if(token.find("f") != std::string::npos) {
                gender = FEMALE;
            }
            else {
                snprintf(messageTx, MAX_LEN, "Unknown gender\n");
                break;
            }
            std::cout << "Token: " << token << "\n";

            std::string filename = languageManager->getWavFilename(language);
            std::cout << "Filename: " << filename << "\n";

            // defaultMessage if ENGLISH, otherwise translate
            std::string newMessage = language == ENGLISH ? message
                : translator->translateToLanguage(message, language);

            pthread_mutex_lock(&audioMixer->fileAccessMutex);
            textToSpeech->translateToWave(newMessage, language, filename, gender);
            pthread_mutex_unlock(&audioMixer->fileAccessMutex);

            audioMixer->readWaveFileIntoMemory(filename, language);
            
            snprintf(messageTx, MAX_LEN, "setVoice;\n");
            break;
        }
        case CL1: 
        {
            std::istringstream istream(input);
            std::string token;
            istream >> token;
            istream >> token;

            int digit = std::stoi(token);

            mode = CL1_INTAKE;
            numPacketsLeft = digit;
                
            snprintf(messageTx, MAX_LEN, "CL1 Intake for %d cycles\n", digit);

            // Delete existing custom1 file
            std::string customFilename = languageManager->getWavFilename(CUSTOM_1);
            remove(customFilename.c_str());

            break;
        }
        case CL2: 
        {
            std::istringstream istream(input);
            std::string token;
            istream >> token;
            istream >> token;

            int digit = std::stoi(token);

            mode = CL2_INTAKE;
            numPacketsLeft = digit;
                
            snprintf(messageTx, MAX_LEN, "CL2 Intake for %d cycles\n", digit);

            // Delete existing custom file
            std::string customFilename = languageManager->getWavFilename(CUSTOM_2);
            remove(customFilename.c_str());

            break;
        }
        case SENDING_DATA:
        {
            Language customLanguage = mode == CL1_INTAKE ? CUSTOM_1 : CUSTOM_2;
            std::string customFilename = languageManager->getWavFilename(customLanguage);

            std::ofstream customFile(customFilename, std::ios::binary | std::ios::app); // Open file in append mode
            customFile.write(input, length);
            customFile.close();

            audioMixer->readWaveFileIntoMemory(customFilename, customLanguage);

            snprintf(messageTx, MAX_LEN, "Read custom file into memory\n");

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