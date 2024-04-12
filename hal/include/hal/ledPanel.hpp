// ledPanel.hpp
// 
// Driver file for the ADA420 32x16 LED Panel
// 
// A C++ adaptation of the test sample code originally written by Janet Mardjuki (December 2015)
// and modified by Jasper Wong (March 2022).
// All credit for code which sets up and drives the matrix according to screen[][] goes to the original authors.

#ifndef _LED_PANEL_H_
#define _LED_PANEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <thread>

#include <hal/languageManager.hpp>

/* GPIO Pins Definition */
#define RED1_PIN 8     // UPPER
#define GREEN1_PIN 80
#define BLUE1_PIN 78
#define RED2_PIN 76 // LOWER
#define GREEN2_PIN 79
#define BLUE2_PIN 74
#define CLK_PIN 73      // Arrival of each data
#define LATCH_PIN 75    // End of a row of data
#define OE_PIN 71       // Transition from one row to another
#define A_PIN 72        // Row select
#define B_PIN 77
#define C_PIN 70

#define S_IWRITE "S_IWUSR"

/* TIMING */
#define DELAY_IN_US 5
#define DELAY_IN_SEC 0

#define NUM_ROWS 16
#define NUM_COLS 32

class LEDPanel {
public:
    LEDPanel();
    ~LEDPanel();
    void displayFlag(enum Language language);

private:

    bool isInitialized = false;

    /* LED Screen Values */
    int screen[NUM_COLS][NUM_ROWS];

    /* FILES HANDLER */
    int fileDesc_red1;
    int fileDesc_blue1;
    int fileDesc_green1;
    int fileDesc_red2;
    int fileDesc_blue2;
    int fileDesc_green2;
    int fileDesc_clk;
    int fileDesc_latch;
    int fileDesc_oe;
    int fileDesc_a;
    int fileDesc_b;
    int fileDesc_c;

    // thread to constantly render image
    std::thread renderingThreadID;
    bool isRunning;

    // private methods
    void sleepForMs(long long delayInMs);
    void initializeGPIOPins();
    void exportAndOut(int pin);
    void bitBangClock();
    void latchMatrix();
    void intToRGB(int* arr, int input);
    void setRow(int row);
    void setColourTop(int colour);
    void setColourBottom(int colour);
    void refreshMatrix();
    void* renderThreadFunction(void* arg);
    void drawCanada();
    void drawFrance();
    void drawGermany();
    void drawSpain();
    void drawChina();
    void drawCustom();
    void drawBlank();

};

#endif
