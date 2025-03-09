#include "flurry/cpu/idt.h"
#include <stdbool.h>

#include "flurry/log/tty.h"


InterruptDescriptor idt[256];
static IDTR idtr;

static bool vectors[256];
extern void* isr_stubs[];

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    InterruptDescriptor* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = K_CODE_SELECTOR;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

void idt_init() {
    idtr.base = idt;
    idtr.limit = sizeof(InterruptDescriptor) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stubs[vector], INT_GATE);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag

    kprintf("[IDT] Initialized\n");
}
