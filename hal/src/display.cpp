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


/**
 * Writes the given value to the specified file.
 *
 * @param filename The name of the file to write to.
 * @param value The value to write to the file.
 * @throws std::runtime_error If the file cannot be opened for writing.
 */
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


/**
 * @brief Constructor for the display class.
 * 
 * Initializes the display object by setting the running flag to true and initializing the display digits to 0x00.
 * It also starts a new thread for the display loop.
 * NOTE: This class handles it's own threads. The caller is not required to do anything related to threads.
 */
display::display() {
    running = true;
    display_digits_left = std::make_tuple(0x00, 0x00);
    display_digits_right = std::make_tuple(0x00, 0x00);

    // start a new thread for the display loop
    displayThread = std::thread (&display::display_loop, this);

}


/**
 * @brief Resets the display digits to zero.
 * 
 * This function sets the display digits on the left and right side to zero.
 * It acquires a lock on the mutex to ensure thread safety.
 */
void display::reset_display_to_zero() {
    // get mtx
    std::lock_guard<std::mutex> lock(mtx);
    // change the digits to 0
    display_digits_left = std::make_tuple(0x00, 0x00);
    display_digits_right = std::make_tuple(0x00, 0x00);
}


/**
 * @brief Cleans up the display by resetting it to zero and sleeping for 100 milliseconds.
 */
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




/**
 * Initializes the I2C communication and sets up the display.
 * 
 * @return The file descriptor for the I2C bus.
 */
int display::initI2C() {
    int i2cFileDesc = initI2cBus("/dev/i2c-1", 0x20);  

    wrt("/sys/class/gpio/gpio61/direction", "out");
    wrt("/sys/class/gpio/gpio44/direction", "out");
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);


    // clear displays
    writeI2cReg(i2cFileDesc, REG_OUTA, 0x00);
    writeI2cReg(i2cFileDesc, REG_OUTB, 0x00);
    
    
    return i2cFileDesc;
}



/**
 * @brief Executes the main loop for the display.
 * 
 * This function continuously updates the display by writing values to GPIO pins and I2C registers.
 * It toggles the values of GPIO pins to control the display segments and refreshes the display at a specified rate.
 * The loop runs until the `running` flag is set to false.
 */
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


// static variables
int current_number = 0;


/**
 * @brief Writes a number to the display.
 *
 * This function writes a number to the display by updating the left and right digits.
 * If the given number is the same as the current number, no action is taken.
 * The number is clamped to a maximum value of 99.
 * The left and right digits are obtained by dividing the number by 10 and taking the remainder, respectively.
 * The left and right digits are then retrieved using the `getNumbers` function.
 * The display digits are updated with the new left and right digits.
 *
 * @param i2cFileDesc The file descriptor for the I2C device.
 * @param number The number to be written to the display.
 */
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



/**
 * Initializes the I2C bus for communication with a device at the specified address.
 * 
 * @param bus The path to the I2C bus device.
 * @param address The address of the I2C device.
 * @return The file descriptor of the opened I2C bus.
 */
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

/**
 * @brief Returns a tuple containing two uint8_t values.
 *
 * This function takes a uint8_t number as input and returns a tuple containing two uint8_t values.
 * The first value in the tuple represents the high byte of the number, while the second value represents the low byte.
 *
 * @param number The input number for which the tuple is generated.
 * @return A tuple containing two uint8_t values representing the high byte and low byte of the input number.
 */
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
