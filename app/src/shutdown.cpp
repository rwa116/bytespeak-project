#include <pthread.h>

#include "shutdown.hpp"

ShutdownManager::ShutdownManager() {
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_lock(&lock);
}

ShutdownManager::~ShutdownManager() {
    pthread_mutex_destroy(&lock);
}

void ShutdownManager::signalShutdown(void) {
    pthread_mutex_unlock(&lock);
    timeToShutdown = true;
}

bool ShutdownManager::isShutdown(void) {
    return timeToShutdown;
}

void ShutdownManager::waitForShutdown(void) {
    pthread_mutex_lock(&lock);
    pthread_mutex_unlock(&lock);
}