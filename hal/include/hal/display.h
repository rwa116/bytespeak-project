#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H


#include <tuple>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

class display {
private:
    static const unsigned char REG_DIRA = 0x02;
    static const unsigned char REG_DIRB = 0x03;
    static const unsigned char REG_OUTA = 0x00;
    static const unsigned char REG_OUTB = 0x01;

    // mutex
    std::mutex mtx;
    std::tuple<uint8_t, uint8_t> display_digits_left;
    std::tuple<uint8_t, uint8_t> display_digits_right;

    void display_loop();
    
    std::atomic<bool> running;
    
    // thread for display loop
    std::thread displayThread;

    // file descriptor for i2c
    std::atomic<int> fileDesc;
    std::mutex fileDescMutex;

    int refresh_rate = 4000;

    // to be called during cleanup
    void reset_display_to_zero();

    void cleanup();

public:

    /**
     * @brief Initializes the display.
     *
     * This function initializes the display and prepares it for use.
     * It should be called before any other display-related functions are used. 
     * NOTE: This function will create a new thread to handle display. The caller is not required to do anything related to threads.
     */
    display();

    /**
     * @brief Destroys the display.
     *
     * This function cleans up the display and prepares it for shutdown.
     * It should be called when the display is no longer needed.
     */
    ~display();


    int initI2C();
    void endI2C(int);


    /**
     * Writes a number to the display.
     *
     * @param i2cFileDesc The file descriptor for the I2C device.
     * @param number The number to be written to the display.
     */
    void writeNumber(int i2cFileDesc, uint8_t number);

    int initI2cBus(std::string bus, int address);
    unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
    void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

    std::tuple<uint8_t, uint8_t> getNumbers(uint8_t number);

    void exportPin(int pin);

    void refresh_display(bool isLeft);

};

#endif // I2C_DRIVER_H