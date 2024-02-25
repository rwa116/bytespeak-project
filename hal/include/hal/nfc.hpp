// NFC Module

#ifndef _NFC_H_
#define _NFC_H_

#include <vector>

class NFCBoard {
public:
    NFCBoard();
    void sendCommand(std::vector<unsigned char> data);
    std::vector<unsigned char> receiveData();
    void syncPackets();
    std::vector<unsigned char> getUid();
private:
    int initI2cBus(char* bus, u_int8_t address);
    void writeI2cReg(int i2cFileDesc, unsigned char content[], int length);
    void readI2cReg(int i2cFileDesc, unsigned char *buffer, int length);
    int i2cDescriptor;
};

#endif