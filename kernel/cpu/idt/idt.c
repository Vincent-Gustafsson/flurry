#include "flurry/cpu/idt.h"
#include <stdbool.h>

#include "log.h"
#include "flurry/log/tty.h"

#define K_CODE_SELECTOR 0x08
#define IDT_MAX_DESCRIPTORS 256

#define INT_GATE 0x8E
#define TRAP_GATE 0x8F



typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isr_high;     // The higher 32 bits of the ISR's address
    uint32_t    reserved;     // Set to zero
} __attribute__((packed)) IdtDescriptor;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) Idtr;


static __attribute__((aligned(8))) IdtDescriptor idt[IDT_MAX_DESCRIPTORS];
static Idtr idtr;

extern void* isr_array[];

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    IdtDescriptor* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = K_CODE_SELECTOR;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

void idt_init() {
    idtr.base = (uint64_t) &idt;
    idtr.limit = (uint16_t)sizeof(IdtDescriptor) * IDT_MAX_DESCRIPTORS - 1;

    for (uint16_t vector = 0; vector < IDT_MAX_DESCRIPTORS; vector++)
        idt_set_descriptor(vector, isr_array[vector], 0x8E);

    logln(LOG_INFO, "[IDT] Initialized");
    idt_reload();
    __asm__ volatile("sti");
}

void idt_reload() {
    __asm__ volatile("lidt %0" : : "m"(idtr));
}
