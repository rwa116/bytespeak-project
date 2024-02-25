#include <iostream>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <string>
#include <thread>
#include <chrono>
#include "hal/nfc.hpp"


NFCBoard::NFCBoard() {
    char i2cBus[] = "/dev/i2c-2";
    u_int8_t deviceAddr = 0x24;
    // Initialize I2C2 Bus
    i2cDescriptor = initI2cBus(i2cBus, deviceAddr);
	std::vector<unsigned char> SAMConfiguration = {0x14, 0x01};
	sendCommand(SAMConfiguration);
}

void NFCBoard::sendCommand(std::vector<unsigned char> data) {
	const u_int8_t length = data.size();
	const u_int8_t checksumLcs = ~length + 1;
	u_int8_t tfi = 0xd4;
	u_int8_t checksumDcs = tfi;
	for(auto it = data.begin(); it != data.end(); ++it) {
		checksumDcs += *it;
	}
	unsigned char *contentBuffer = new unsigned char[data.size() + 8];

	// Fill out frame
	contentBuffer[0] = 0x00;
	contentBuffer[1] = 0x00;
	contentBuffer[2] = 0xFF;
	contentBuffer[3] = length;
	contentBuffer[4] = checksumLcs;
	contentBuffer[5] = tfi;
	int ind = 6;
	for(auto it = data.begin(); it != data.end(); ++it) {
		contentBuffer[ind] = *it;
		ind++;
	}
	contentBuffer[ind] = ~checksumDcs + 1;
	contentBuffer[ind + 1] = 0x00;

	std::cout << "Size = " << data.size() << std::endl;
	writeI2cReg(i2cDescriptor, contentBuffer, data.size() + 8);

	delete[] contentBuffer;
}

std::vector<unsigned char> NFCBoard::receiveData() {
	std::vector<unsigned char> data;
	for(int i=0; i<10; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		unsigned char buffer[128] = {0};

		readI2cReg(i2cDescriptor, buffer, 8);

		//Check the received data
		int index = 0;
		for(int j = 0; j < 128; j++) {
			switch(j) {
				case 0:
				case 1:
					if(buffer[j] == 0x00) {
						index++;
						continue;
					} else {
						index = 0;
					}
					break;
				case 2:
					if(buffer[j] == 0xFF) {
						index++;
						continue;
					}
					else {
						std::cerr << "Ack Response Err2" << std::endl;
						exit(-1);
					}
				case 3:
					if(buffer[j] == 0x01) {
						std::cerr << "App Err2" << std::endl;
						exit(-1);
					} else {
						u_int8_t size = buffer[j];
						std::vector<unsigned char> data(buffer + j + 3, buffer + j + 3 + size);
						std::cout << "Received Data: " << std::endl;
						return data;
					}
				default:
					index = 0;
					break;
			}
		}
	}
	std::cerr << "Timeout receiving data" << std::endl;
	exit(-1);
}

void NFCBoard::syncPackets() {
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	for(int i=0; i<5; i++){
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
		unsigned char buffer[128] = {0};

		readI2cReg(i2cDescriptor, buffer, 32);
		for(int i=0; i<128; i++) {
			std::cout << " " << std::hex << unsigned(buffer[i]);
		}
		std::cout << std::endl;
		int index = 0;

		for(int j = 0; j < 128; j++) {
			switch(j) {
				case 0:
				case 1:
					if(buffer[j] == 0x00) {
						index++;
						continue;
					} else {
						index = 0;
					}
					break;
				case 2:
					if(buffer[j] == 0xFF) {
						index++;
						continue;
					}
					else {
						std::cerr << "Ack Response Err1" << std::endl;
						exit(-1);
					}
					break;
				case 3:
					if(buffer[j] == 0x00) {
						return;
					} else if(buffer[j] == 0x01) {
						std::cerr << "App Err1" << std::endl;
						exit(-1);
					}
					break;
				default:
					index = 0;
					break;
			}
		}
	}
	std::cerr << "Timeout receiving data" << std::endl;
	exit(-1);
}

std::vector<unsigned char> NFCBoard::getUid() {
	std::vector<unsigned char> passiveOneTarget = {0x4A, 0x01, 0x00};
	sendCommand(passiveOneTarget);

	syncPackets();
	std::vector<unsigned char> response = receiveData();
	int responseLength = response.size();
	int index = 6;
	std::vector<unsigned char> id;
	int idLength = response[index];
	index++;
	if(index + idLength > responseLength) {;
		return id;
	}
	std::vector<unsigned char> uid(response.begin() + index, response.begin() + index + idLength);
	return uid;
}

int NFCBoard::initI2cBus(char *bus, u_int8_t address)
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

void NFCBoard::writeI2cReg(int i2cFileDesc, unsigned char content[], int length)
{
	int res = write(i2cFileDesc, content, length);
	if (res != length) {
		std::cerr << "Unable to write i2c register" << std::endl;
		exit(-1);
	}
}

void NFCBoard::readI2cReg(int i2cFileDesc, unsigned char *buffer, int length)
{
	for(int i=0; i<length; i++) {
		// To read a register, must first write the address
		int res = write(i2cFileDesc, &buffer[i], 1);
		if (res != 1) {
			std::cerr << "Unable to write i2c register" << std::endl;
			exit(-1);
		}
		
		// Now read the value and return it
		res = read(i2cFileDesc, buffer + i, 1);
		if (res != 1) {
			std::cerr << "Unable to read i2c register" << std::endl;
			exit(-1);
		}
	}
}