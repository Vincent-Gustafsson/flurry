#include "flurry/common.h"
#include "flurry/log/tty.h"

void kpanic() {
    asm volatile("cli");
    kprintf("KERNEL PANIC\n");

    // Halt the system
    for (;;) {
        __asm__ volatile("hlt");
    }
}
