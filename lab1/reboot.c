#include "reboot.h"

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {           // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // set watchdog reset config to full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // set watchdog reset config to no reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}