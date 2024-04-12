// ADA420 LED Matrix Driver
// 
// Large chunk of implementations to set up pins and drive led matrix are attributed to Janet Mardjuki and Jasper Wong

#include "hal/ledPanel.hpp"

LEDPanel::LEDPanel()
{
    assert(!isInitialized);
    memset(screen, 0, sizeof(screen));
    initializeGPIOPins();
    drawCanada();
    renderingThreadID = std::thread([this]() {
        this->renderThreadFunction(nullptr);
    });
    isRunning = true;
    isInitialized = true;
}

LEDPanel::~LEDPanel()
{
    assert(isInitialized);
    drawBlank();
    refreshMatrix();
    isRunning = false;
    renderingThreadID.join();
    isInitialized = false;
}

void LEDPanel::displayFlag(enum Language language)
{
    if (isTranslationOccuring){
        return;
    }

    switch(language){
        case ENGLISH:
            // draw Canada flag
            drawCanada();
            break;
        case FRENCH:
            // draw French flag
            drawFrance();
            break;
        case GERMAN:
            // draw German flag
            drawGermany();
            break;
        case SPANISH:
            // draw Spanish flag
            drawSpain();
            break;
        case CHINESE:
            // draw Chinese flag
            drawChina();
            break;
        default:
            drawCustom();
            // draw something unique
    }
}

void LEDPanel::triggerTranslationStatus(bool startingTranslation)
{
    if (startingTranslation){
        drawBlank();
        drawLoading();
    }
    isTranslationOccuring = startingTranslation;
    
}

void LEDPanel::sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *)NULL);
}

// From sample code written by Janet Mardjuki and Jasper Wong
void LEDPanel::initializeGPIOPins()
{
    // !Upper led
    exportAndOut(RED1_PIN);
    fileDesc_red1 = open("/sys/class/gpio/gpio8/value", O_WRONLY, S_IWRITE);
    exportAndOut(GREEN1_PIN);
    fileDesc_green1 = open("/sys/class/gpio/gpio80/value", O_WRONLY, S_IWRITE);
    exportAndOut(BLUE1_PIN);
    fileDesc_blue1 = open("/sys/class/gpio/gpio78/value", O_WRONLY, S_IWRITE);

    // Lower led
    exportAndOut(RED2_PIN);
    fileDesc_red2 = open("/sys/class/gpio/gpio76/value", O_WRONLY, S_IWRITE);
    exportAndOut(GREEN2_PIN);
    fileDesc_green2 = open("/sys/class/gpio/gpio79/value", O_WRONLY, S_IWRITE);
    exportAndOut(BLUE2_PIN);
    fileDesc_blue2 = open("/sys/class/gpio/gpio74/value", O_WRONLY, S_IWRITE);

    // Timing
    exportAndOut(CLK_PIN);
    fileDesc_clk = open("/sys/class/gpio/gpio73/value", O_WRONLY, S_IWRITE);
    exportAndOut(LATCH_PIN);
    fileDesc_latch = open("/sys/class/gpio/gpio75/value", O_WRONLY, S_IWRITE);
    exportAndOut(OE_PIN);
    fileDesc_oe = open("/sys/class/gpio/gpio71/value", O_WRONLY, S_IWRITE);

    // Row Select
    exportAndOut(A_PIN);
    fileDesc_a = open("/sys/class/gpio/gpio72/value", O_WRONLY, S_IWRITE);
    exportAndOut(B_PIN);
    fileDesc_b = open("/sys/class/gpio/gpio77/value", O_WRONLY, S_IWRITE);
    exportAndOut(C_PIN);
    fileDesc_c = open("/sys/class/gpio/gpio70/value", O_WRONLY, S_IWRITE); 
}

void LEDPanel::exportAndOut(int pin)
{
    char fileNameBuffer[1024];
    sprintf(fileNameBuffer, "/sys/class/gpio/gpio%d/direction", pin);
    FILE *gpioDirP = fopen(fileNameBuffer, "w");

    // Check for whether the pin is exported
    if (gpioDirP == NULL) {
        // If the pointer is NULL, the pin has not been exported, so export it
        FILE *pFile = fopen("/sys/class/gpio/export" , "w");
        if (pFile == NULL) {
            printf("ERROR: Unable to open export file.\n");
            exit(1);
        }
        fprintf(pFile, "%d", pin);
        fclose(pFile);
        sleepForMs(330);

        // Pin should now be exported, so reassign the gpioDirP file pointer
        gpioDirP = fopen(fileNameBuffer, "w"); // okay to be reassigned as it was NULL
    }

    // Now set the direction to out
    fprintf(gpioDirP, "out");
    fclose(gpioDirP);
}

void LEDPanel::bitBangClock()
{
    // Bit-bang the clock gpio
    // Notes: Before program writes, must make sure it's on the 0 index
    lseek(fileDesc_clk, 0, SEEK_SET);
    write(fileDesc_clk, "1", 1);
    lseek(fileDesc_clk, 0, SEEK_SET);
    write(fileDesc_clk, "0", 1);
}

void LEDPanel::latchMatrix()
{
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "1", 1);
    lseek(fileDesc_latch, 0, SEEK_SET);
    write(fileDesc_latch, "0", 1);
}

// TODO: change this, see AdaFruit driver - https://github.com/adafruit/RGB-matrix-Panel/tree/master
void LEDPanel::intToRGB(int* arr, int input)
{
    arr[0] = input & 1;

    arr[1] = input & 2;
    arr[1] = arr[1] >> 1;

    arr[2] = input & 4;
    arr[2] = arr[2] >> 2;
}

void LEDPanel::setRow(int row)
{
    // Convert rowNum single bits from int
    int arr[3] = {0, 0, 0};
    intToRGB(arr, row);

    // Write on the row pins
    char a_val[2];
    sprintf(a_val, "%d", arr[0]);
    lseek(fileDesc_a, 0, SEEK_SET);
    write(fileDesc_a, a_val, 1);

    char b_val[2];
    sprintf(b_val, "%d", arr[1]);
    lseek(fileDesc_b, 0, SEEK_SET);
    write(fileDesc_b, b_val, 1);

    char c_val[2];
    sprintf(c_val, "%d", arr[2]);
    lseek(fileDesc_c, 0, SEEK_SET);
    write(fileDesc_c, c_val, 1);    
}

void LEDPanel::setColourTop(int colour)
{
    int arr[3] = {0, 0, 0};
    intToRGB(arr, colour);

    // Write on the colour pins
    char red1_val[2];
    sprintf(red1_val, "%d", arr[0]);
    lseek(fileDesc_red1, 0, SEEK_SET);
    write(fileDesc_red1, red1_val, 1);

    char green1_val[2];
    sprintf(green1_val, "%d", arr[1]);
    lseek(fileDesc_green1, 0, SEEK_SET);
    write(fileDesc_green1, green1_val, 1);

    char blue1_val[2];
    sprintf(blue1_val, "%d", arr[2]);
    lseek(fileDesc_blue1, 0, SEEK_SET);
    write(fileDesc_blue1, blue1_val, 1);   
}

void LEDPanel::setColourBottom(int colour)
{
    int arr[3] = {0,0,0};
    intToRGB(arr, colour);

    // Write on the colour pins
    char red2_val[2];
    sprintf(red2_val, "%d", arr[0]);
    lseek(fileDesc_red2, 0, SEEK_SET);
    write(fileDesc_red2, red2_val, 1);

    char green2_val[2];
    sprintf(green2_val, "%d", arr[1]);
    lseek(fileDesc_green2, 0, SEEK_SET);
    write(fileDesc_green2, green2_val, 1);

    char blue2_val[2];
    sprintf(blue2_val, "%d", arr[2]);
    lseek(fileDesc_blue2, 0, SEEK_SET);
    write(fileDesc_blue2, blue2_val, 1);    
}

void LEDPanel::refreshMatrix()
{
    for ( int rowNum = 0; rowNum < 8; rowNum++ ) {
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "1", 1); 
        setRow(rowNum);
        for ( int colNum = 0; colNum < 32;  colNum++) {
            setColourTop(screen[colNum][rowNum]);
            setColourBottom(screen[colNum][rowNum+8]);
            bitBangClock();
        }
        latchMatrix();
        lseek(fileDesc_oe, 0, SEEK_SET);
        write(fileDesc_oe, "0", 1); 
        struct timespec reqDelay = {DELAY_IN_SEC, DELAY_IN_US}; // sleep for delay
    	nanosleep(&reqDelay, (struct timespec *) NULL);
    }
}

void* LEDPanel::renderThreadFunction(void* arg)
{
    (void)arg;
    while (isRunning) {
        refreshMatrix();
    }
    return nullptr;
}

void LEDPanel::drawCanada()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            if (x <= 6 || x >= 24){
                screen[x][y] = 1; //red
            } else {
                screen[x][y] = 7; //white
            }
        }
    }
    //col 11
    for (int y = 6; y <= 8; y++){
        screen[11][y] = 1;
    }
    for (int y = 6; y <= 11; y++){
        if (y != 10)
            screen[12][y] = 1;
    }
    for (int y = 7; y <= 10; y++){
        screen[13][y] = 1;
    }
    for (int y = 4; y <= 10; y++){
        screen[14][y] = 1;
    }
    for (int y = 3; y <= 12; y++){
        screen[15][y] = 1;
    }
    for (int y = 4; y <= 10; y++){
        screen[16][y] = 1;
    }
    for (int y = 7; y <= 10; y++){
        screen[17][y] = 1;
    }
    for (int y = 6; y <= 11; y++){
        if (y != 10)
            screen[18][y] = 1;
    }
    for (int y = 6; y <= 8; y++){
        screen[19][y] = 1;
    }
}

void LEDPanel::drawFrance()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            if (x <= 9){
                screen[x][y] = 4; //blue
            } else if (x >= 22) {
                screen[x][y] = 1; //red
            } else {
                screen[x][y] = 7; //white
            }
        }
    }
}

void LEDPanel::drawGermany()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            if (y <= 5){
                screen[x][y] = 0; //black
            } else if (y <= 10) {
                screen[x][y] = 1; //red
            } else {
                screen[x][y] = 3; //yellow
            }
        }
    }
}

void LEDPanel::drawSpain()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            if (y <= 3 || y >= 12){
                screen[x][y] = 1; //red
            } else {
                screen[x][y] = 3; //yellow
            }
        }
    }
    screen[2][5] = 4;
    screen[2][10] = 4;
    screen[9][5] = 4;
    screen[9][10] = 4;
    screen[2][7] = 7;
    screen[2][9] = 7;
    screen[9][7] = 7;
    screen[9][9] = 7;
    screen[5][5] = 1;
    screen[6][5] = 1;
    screen[5][10] = 7;
    screen[6][10] = 1;
    for (int x = 2; x <= 9; x++){
        if (x == 6 || x == 7){
            screen[x][6] = 7;
        } else {
            screen[x][6] = 1;
        }
    }
    for (int x = 4; x <= 7; x++){
        if (x == 6 || x == 7){
            screen[x][7] = 7;
        } else {
            screen[x][7] = 1;
        }
    }
    for (int x = 2; x <= 9; x++){
        if (x == 4 || x == 5){
            screen[x][8] = 7;
        } else {
            screen[x][8] = 1;
        }
    }
    for (int x = 4; x <= 7; x++){
        if (x == 4 || x == 5){
            screen[x][9] = 7;
        } else {
            screen[x][9] = 1;
        }
    }
}

void LEDPanel::drawChina()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            screen[x][y] = 1; //red
        }
    }
    //colour some additional pixels
    screen[9][1] = 3;
    screen[11][3] = 3;
    screen[11][6] = 3;
    screen[9][8] = 3;
    for (int y = 2; y <= 5; y++){
        screen[4][y] = 3;
    }
    for (int x = 2; x <= 6; x++){
        screen[x][4] = 3;
    }
    screen[3][5] = 3;
    screen[3][6] = 3;
    screen[5][5] = 3;
    screen[5][6] = 3;
}

void LEDPanel::drawCustom()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            screen[x][y] = x;
        }
    }
}

void LEDPanel::drawBlank()
{
    for (int y = 0; y < NUM_ROWS; y++){
        for (int x = 0; x < NUM_COLS; x++){
            screen[x][y] = 0; //off
        }
    }
}

void LEDPanel::drawLoading()
{
    for (int y = 0; y < 8; y++){
        if (y % 2 == 0){
            screen[0][y] = 7;
        }
    }
}