#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <tuple>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

class Display {
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

    int refresh_rate = 100000;

    // to be called during cleanup
    void reset_display_to_zero();

    void cleanup();

public:
    Display();
    ~Display();

    int initI2C();
    void endI2C(int);

    int getDesc();


    void writeNumber(int i2cFileDesc, uint8_t number);

    int initI2cBus(std::string bus, int address);
    unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
    void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

    std::tuple<uint8_t, uint8_t> getNumbers(uint8_t number);
};

#endif // I2C_DRIVER_H