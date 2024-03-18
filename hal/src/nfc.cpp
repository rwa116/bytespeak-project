#include "hal/nfc.hpp"

NFCReader::NFCReader(const char* device, int address) 
    : device(device), fileDescriptor(-1), address(address) {
    fileDescriptor = initI2C();
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
        return true;
    } else {
        std::cerr << "Failed to read from the device." << std::endl;
        return false;
    }
}

std::string NFCReader::waitForCardAndReadUID() {
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

        0xE1, // Checksum for the command bytes (0x200 - 0xD4 - 0x4a - 0x01 - 0x00 = 0xE1)
        0x00, // Reserved byte
    };
    unsigned char response[256];
    std::string uid;


    memset(response, 0, sizeof(response));

    if (sendCommandAndWaitForResponse(setupCommand, sizeof(setupCommand), response, sizeof(response))) {
        for(int i = 0; i < 23; i++) {
            std::cout << "0x" << std::hex << (int)response[i] << " ";
        }
        std::cout << std::endl;
    }

    while (true) {

        memset(response, 0, sizeof(response));

        if (sendCommandAndWaitForResponse(detectCardCommand, sizeof(detectCardCommand), response, sizeof(response))) {
            std::cout << "Response" << std::endl;
            for(int i = 0; i < 23; i++) {
                std::cout << "0x" << std::hex << (int)response[i] << " ";
            }
            std::cout << std::endl;
            for (int i = 19; i < 23; i++) { // Assuming 4-byte UID
                uid += (i > 19 ? ":" : "") + std::to_string((int)response[i]);
            }
            break;
        }
        sleep(1);
    }

    return uid;
}
