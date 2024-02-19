#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string>

#include "hal/nfc.hpp"

int initI2cBus(char* bus, int address);


NFCBoard::NFCBoard() {
    char i2cBus[] = "/dev/i2c-2";
    int deviceAddr = 0x24;
    // Initialize I2C2 Bus
    i2cDescriptor = initI2cBus(i2cBus, deviceAddr);
}

int NFCBoard::initI2cBus(char *bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		std::cout << "I2C DRV: Unable to open bus for read/write " << bus << std::endl;;
		std::cerr << "Error is:" << std::endl;
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		std::cerr << "Unable to set I2C device to slave address." << std::endl;
		exit(-1);
	}
	return i2cFileDesc;
}

void NFCBoard::writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		std::cerr << "Unable to write i2c register" << std::endl;
		exit(-1);
	}
}

unsigned char NFCBoard::readI2cReg(int i2cFileDesc, unsigned char regAddr)
{
	// To read a register, must first write the address
	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
	if (res != sizeof(regAddr)) {
		std::cerr << "Unable to write i2c register." << std::endl;
		exit(-1);
	}

	// Now read the value and return it
	char value = 0;
	res = read(i2cFileDesc, &value, sizeof(value));
	if (res != sizeof(value)) {
		std::cerr << "Unable to read i2c register" << std::endl;
		exit(-1);
	}
	return value;
}