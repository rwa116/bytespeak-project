#ifndef _PRU_DRIVER_HPP_
#define _PRU_DRIVER_HPP_

#include "sharedDataStruct.h"

class PruDriver {
public:
    PruDriver();
    ~PruDriver();
    bool isRightPressed();
    bool isDownPressed();
private:
    volatile sharedMemStruct_t *pSharedPru0;

    volatile void* getPruMmapAddr(void);
    void freePruMmapAddr(volatile void* pPruBase);
};

#endif