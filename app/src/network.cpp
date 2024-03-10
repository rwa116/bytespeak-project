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

#include "network.hpp"

Network::Network() {
    isRunning = true;

    networkThreadId = std::thread([this]() {
        this->networkThread(nullptr);
    });
}

Network::Network(ShutdownManager *shutdownInstance, TextToSpeech *textInstance, AudioMixer *audioInstance) {
    isRunning = true;
    shutdownManager = shutdownInstance;
    textToSpeech = textInstance;
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
    char* token = strtok(input, " ");
    if(strtok(token, "epseak") == 0) {
        return ESPEAK;
    }
    else if(strtok(input, "terminate") == 0) {
        return TERMINATE;
    }
    else {
        return UNKNOWN;
    }
}

void Network::sendReply(enum Command command, char *input, int socketDescriptor, struct sockaddr_in *sinRemote) {
    char messageTx[MAX_LEN];
    unsigned int sinLen;

    switch(command) {
        case ESPEAK:
            (void)input;
            snprintf(messageTx, MAX_LEN, "Set message to %s\n", "PLACEHOLDER");
            break;
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