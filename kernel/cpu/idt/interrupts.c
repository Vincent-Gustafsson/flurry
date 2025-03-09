#include "flurry/cpu/interrupts.h"
#include "flurry/log/tty.h"



__attribute__((noreturn))
void interrupt_dispatch(InterruptFrame *frame) {
    kprintf("PANIC: ");
    switch (frame->interrupt_number) {
        case 0:
            kprintf("Divide By Zero\n");
            break;
        case 13:
            kprintf("General Protection Fault\n");
        break;
        case 14:
            kprintf("Page Fault\n");
            break;
        default:
            kprintf("IDK");
    }
    __asm__ volatile ("cli; hlt");
}