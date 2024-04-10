/*
    implementation for the display
*/

#include "hal/display.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <tuple>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <thread>

inline static void wrt(std::string filename, std::string value) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Unable to open file (" << filename << ") for write\n";
        throw std::runtime_error("Error opening file for writing");
    }
    file << value;
    if (!file) {
        std::cerr << "ERROR WRITING DATA\n";
    }

}


display::display() {
    running = true;
    display_digits_left = std::make_tuple(0x00, 0x00);
    display_digits_right = std::make_tuple(0x00, 0x00);


    this->fileDesc = initI2C();
    // start a new thread for the display loop
    displayThread = std::thread (&display::display_loop, this);

}


void display::reset_display_to_zero() {
    // get mtx
    std::lock_guard<std::mutex> lock(mtx);
    // change the digits to 0
    display_digits_left = std::make_tuple(0x00, 0x00);
    display_digits_right = std::make_tuple(0x00, 0x00);
}

void display::cleanup() {
    reset_display_to_zero();
    // sleep for 100 milliseconds
    usleep(100000);
}

void display::endI2C(int i2cFileDesc) {
    if (running){
    
        cleanup();
        running = false;
        displayThread.join();
    }
    close(i2cFileDesc);
}


// destructor
display::~display() {
    if (running){
        cleanup();
        
        running = false;
        displayThread.join();
    }
}


int display::initI2C() {
    int i2cFileDesc = initI2cBus("/dev/i2c-1", 0x20);

    // turn on displays
    wrt("/sys/class/gpio/gpio61/direction", "out");
    wrt("/sys/class/gpio/gpio44/direction", "out");
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);


    // clear displays
    writeI2cReg(i2cFileDesc, REG_OUTA, 0x00);
    writeI2cReg(i2cFileDesc, REG_OUTB, 0x00);
    
    return i2cFileDesc;
}

int display::getDesc() {
    return fileDesc;
}


void display::display_loop() {
    wrt("/sys/class/gpio/gpio61/value", "1");
    wrt("/sys/class/gpio/gpio44/value", "1");

    while (running){
        // refresh_display(true);
        
        // std::lock_guard<std::mutex> lock(mtx);
        wrt("/sys/class/gpio/gpio44/value", "0");

        writeI2cReg(fileDesc, REG_OUTA, std::get<0>(display_digits_left));

        writeI2cReg(fileDesc, REG_OUTB, std::get<1>(display_digits_left));
        wrt("/sys/class/gpio/gpio61/value", "1");
       
        usleep(refresh_rate);

        
        // refresh_display(false);

        wrt("/sys/class/gpio/gpio61/value", "0");
        wrt("/sys/class/gpio/gpio44/value", "0");

        writeI2cReg(fileDesc, REG_OUTA, std::get<0>(display_digits_right));
        writeI2cReg(fileDesc, REG_OUTB, std::get<1>(display_digits_right));

        wrt("/sys/class/gpio/gpio44/value", "1");
        usleep(refresh_rate); 
    }
}

int current_number = -1;

void display::writeNumber(int i2cFileDesc, uint8_t number) {

    
    if (number == current_number) {
        return;
    }

    current_number = number;
    int newNum = number > 99 ? 99 : number;

    uint8_t leftNumber = newNum / 10;
    uint8_t rightNumber = newNum % 10;

    auto leftDigits = getNumbers(leftNumber);
    auto rightDigits = getNumbers(rightNumber);
    

    std::lock_guard<std::mutex> lock(mtx);

    display_digits_left = leftDigits;
    display_digits_right = rightDigits;

    fileDesc = i2cFileDesc;

}



int display::initI2cBus(std::string bus, int address) {
    int i2cFileDesc = open(bus.c_str(), O_RDWR);
    if (i2cFileDesc < 0) {
        perror("I2C: Unable to open bus for read/write");
        exit(1);
    }

    if (ioctl(i2cFileDesc, I2C_SLAVE, address) < 0) {
        perror("I2C: Unable to set I2C device to slave address");
        exit(1);
    }
    

    return i2cFileDesc;
}




unsigned char display::readI2cReg(int i2cFileDesc, unsigned char regAddr) {

    if (write(i2cFileDesc, &regAddr, 1) != 1) {
        perror("I2C: Unable to write to i2c register");
        exit(1);
    }

    unsigned char value;
    if (read(i2cFileDesc, &value, 1) != 1) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }

    return value;
}

void display::writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value) {
 	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;

	int res = write(i2cFileDesc, buff, 2);

	if (res != 2) {
		perror("I2C: Unable to write i2c register.");
		exit(1);
	}
}

std::tuple<uint8_t, uint8_t> display::getNumbers(uint8_t number) {
    

    switch(number) {                       /*A*/ /*B*/
            case 0: return std::make_tuple(0xD0, 0xA1);
            case 1: return std::make_tuple(0x02, 0x08);
            case 2: return std::make_tuple(0x98, 0x83);
            case 3: return std::make_tuple(0xD8, 0x03);
            case 4: return std::make_tuple(0xC8, 0x22);
            case 5: return std::make_tuple(0x58, 0x23);
            case 6: return std::make_tuple(0x58, 0xA3);
            case 7: return std::make_tuple(0x01, 0x05);
            case 8: return std::make_tuple(0xD8, 0xA3);
            case 9: return std::make_tuple(0xD8, 0x23);
            default: return std::make_tuple(0xD0, 0xA1);
        }
}

void display::exportPin(int pin) {
    std::ofstream pfile("/sys/class/gpio/export");
    if (!pfile.is_open()) {
        throw std::runtime_error("ERROR: Unable to open /sys/class/gpio/export");
    }
    pfile << pin;
    if (pfile.fail()) {
        throw std::runtime_error("ERROR: Writing data to /sys/class/gpio/export");
    }
}


void display::refresh_display(bool isLeft) {
    switch (isLeft) {
        case true: 
            wrt("/sys/class/gpio/gpio61/value", "1");
            wrt("/sys/class/gpio/gpio44/value", "0");
            break;
        case false: 
            wrt("/sys/class/gpio/gpio61/value", "0");
            wrt("/sys/class/gpio/gpio44/value", "1");
            break;
    }
}
