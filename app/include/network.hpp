// Network module
// Acts as a server to serve UDP requests from the client regarding
// requests for details about the sampler (light intensity sensor)

#ifndef _NETWORK_HPP_
#define _NETWORK_HPP_

#include <thread>
#include <shutdown.hpp>
#include <textToSpeech.hpp>
#include <translator.hpp>
#include <hal/audioMixer.hpp>
#include <hal/languageManager.hpp>

enum Command {
    ESPEAK,
    TERMINATE,
    UNKNOWN
};

class Network {
public:
    Network();
    Network(ShutdownManager *shutdownInstance, LanguageManager *languageManagerInstance, TextToSpeech *textInstance, 
            Translator *translatorInstance, AudioMixer *audioInstance);
    ~Network();
private:
    std::thread networkThreadId;
    ShutdownManager *shutdownManager;
    LanguageManager *languageManager;
    TextToSpeech *textToSpeech;
    Translator *translator;
    AudioMixer *audioMixer;

    bool isRunning = false;
    void *networkThread(void *arg);
    enum Command checkCommand(char* input);
    void sendReply(enum Command command, char *input, int socketDescriptor, struct sockaddr_in *sinRemote);
};

#endif