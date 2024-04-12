#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"
#include "sharedDataStruct.h"

#define STR_LEN         8       // # LEDs in our string
#define oneCyclesOn     700/5   // Stay on 700ns
#define oneCyclesOff    800/5
#define zeroCyclesOn    350/5
#define zeroCyclesOff   600/5
#define resetCycles     60000/5 // Must be at least 50u, use 60u
#define sleepCycles     10000

// P8_11 for output (on R30), PRU0
#define DATA_PIN 15       // Bit number to output on

volatile register uint32_t __R30;
volatile register uint32_t __R31;

// GPIO Input: P8_15 = pru0_pru_r31_15 
//   = JSRT (Joystick Right) on Zen Cape
#define JOYSTICK_RIGHT_MASK (1 << 15)
// GPIO Input: P8_16 = pru0_pru_r31_14
//   = JSDN (Joystick Down) on Zen Cape
#define JOYSTICK_DOWN_MASK (1 << 14)


// Shared Memory Configuration
// -----------------------------------------------------------
#define THIS_PRU_DRAM       0x00000         // Address of DRAM
#define OFFSET              0x200           // Skip 0x100 for Stack, 0x100 for Heap (from makefile)
#define THIS_PRU_DRAM_USABLE (THIS_PRU_DRAM + OFFSET)

void main(void)
{
    // This works for both PRU0 and PRU1 as both map their own memory to 0x0000000
    volatile sharedMemStruct_t *pSharedMemStruct = (volatile void *)THIS_PRU_DRAM_USABLE;
    // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

    //Initialize
    pSharedMemStruct->isRightPressed = false;
    pSharedMemStruct->isDownPressed = false;

    while(true) {
        __delay_cycles(sleepCycles);

        // Get Joystick Information
        pSharedMemStruct->isRightPressed = (__R31 & JOYSTICK_RIGHT_MASK) == 0;
        pSharedMemStruct->isDownPressed = (__R31 & JOYSTICK_DOWN_MASK) == 0;
    }

    __halt();
}
