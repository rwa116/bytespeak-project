// Shutdown Module
// This module is responsible for synchronizing the shutdown of the program across all
// modules and threads. Main will be waiting in a function in this module until another
// thread from another module unlocks it (initiates shutdown)

#ifndef _SHUTDOWN_H_
#define _SHUTDOWN_H_

#include <stdbool.h>
#include <pthread.h>

class ShutdownManager {
public:
    ShutdownManager();
    ~ShutdownManager(void);

    void signalShutdown(void);
    bool isShutdown(void);
    void waitForShutdown(void);
private:
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    bool timeToShutdown = false;
};

#endif