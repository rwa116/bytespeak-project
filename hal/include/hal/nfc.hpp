#ifndef NFCREADER_H
#define NFCREADER_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <iomanip> // For std::setw and std::setfill

#include "shutdown.hpp"

class NFCReader {
public:
    NFCReader(const char* device, int address, ShutdownManager* shutdownManager);
    ~NFCReader();
    std::string waitForCardAndReadUID();

private:
    const char* device; // Assuming this is declared first
    int fileDescriptor;
    int address;
    ShutdownManager* shutdownManager;
    
    int initI2C();
    bool sendCommandAndWaitForResponse(unsigned char* command, int commandLength, unsigned char* response, int responseLength, bool need_response);
    bool clearBuffer();

};

#endif // NFCREADER_H
