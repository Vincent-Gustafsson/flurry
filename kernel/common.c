#include "flurry/common.h"

#include "log.h"
#include "flurry/log/tty.h"

void kpanic() {
    asm volatile("cli");
    logln(LOG_ERROR, "KERNEL PANIC");

    // Halt the system
    for (;;) {
        __asm__ volatile("hlt");
    }
}

uint64_t clamp(uint64_t value, uint64_t min, uint64_t max) {
    if (value < min)
        return min;

    if (value > max)
        return max;

    return value;
}
