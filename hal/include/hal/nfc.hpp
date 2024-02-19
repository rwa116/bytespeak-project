// NFC Module

#ifndef _NFC_H_
#define _NFC_H_

class NFCBoard {
public:
    NFCBoard();
private:
    int initI2cBus(char* bus, int address);
    void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
    unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
    int i2cDescriptor;
};

#endif