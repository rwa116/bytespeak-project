#include "hal/nfc.hpp"

NFCReader::NFCReader(const char* device, int address) 
    : device(device), fileDescriptor(-1), address(address) {
    fileDescriptor = initI2C();

    unsigned char response[256];
    unsigned char setupCommand[] = {
        0x00, // Reserved byte
        0x00, // Reserved byte
        0xFF, // Start of packet
        0x03, // Packet length
        0xFD, // Length checksum (0x100 - 0x03 = 0xFD)
        0xD4, // Data Exchange command

        0x14, // SAMConfiguration command
        0x01, // 
        
        0x17, // Checksum for the command bytes (0x100 - 0xD4 - 0x14 - 0x01 = 0x17)
        0x00, // Reserved byte
    };

    if (sendCommandAndWaitForResponse(setupCommand, sizeof(setupCommand), response, sizeof(response))) {
        for(int i = 0; i < 23; i++) {
            std::cout << "0x" << std::hex << (int)response[i] << " ";
        }
        std::cout << std::endl;
    }
}

NFCReader::~NFCReader() {
    if (fileDescriptor >= 0) {
        close(fileDescriptor);
    }
}

int NFCReader::initI2C() {
    int file;
    if ((file = open(device, O_RDWR)) < 0) {
        std::cerr << "Failed to open the I2C bus" << std::endl;
        return -1;
    }
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        std::cerr << "Failed to acquire bus access and/or talk to slave." << std::endl;
        close(file);
        return -1;
    }
    return file;
}

bool NFCReader::sendCommandAndWaitForResponse(unsigned char* command, int commandLength, unsigned char* response, int responseLength) {
    if (write(fileDescriptor, command, commandLength) != commandLength) {
        std::cerr << "Failed to write to the I2C bus." << std::endl;
        return false;
    }

    usleep(250000); 

    if (read(fileDescriptor, response, responseLength) > 0) {
        clearBuffer();
        return true;
    } else {
        std::cerr << "Failed to read from the device." << std::endl;
        return false;
    }
}

bool NFCReader::clearBuffer() {

    return true;
}

std::string NFCReader::waitForCardAndReadUID() {
    unsigned char detectCardCommand[] = {
        0x00, // Reserved byte
        0x00, // Reserved byte
        0xFF, // Start of packet
        0x04, // Packet length
        0xFC, // Length checksum (0x100 - 0x04 = 0xFC)
        0xD4, // Data Exchange command
        0x4A, // InListPassiveTarget command
        0x01, // Max number of targets
        0x00, // Baud rate
        0xE1, // Checksum for the command bytes (0x200 - 0xD4 - 0x4A - 0x01 - 0x00 = 0xE1)
        0x00, // Reserved byte
    };

    unsigned char response[256];
    std::string uid;

    while (true) {
        if (sendCommandAndWaitForResponse(detectCardCommand, sizeof(detectCardCommand), response, sizeof(response))) {
            // Check for a valid response length before processing
            if (response[3] > 0) { // assuming response[3] contains the length of the coming data
                int uidStartIndex = 8; // The index where UID starts in the response might need adjustment
                int uidLength = 4; // Adjust based on actual UID length
                std::stringstream ss;
                for (int i = uidStartIndex; i < uidStartIndex + uidLength; i++) {
                    ss << std::hex << std::setfill('0') << std::setw(2) << (int)response[i];
                    if (i < uidStartIndex + uidLength - 1) {
                        ss << ":";
                    }
                }
                uid = ss.str();
                break;
            }
        }
        sleep(1); 
    }

    return uid;
}


