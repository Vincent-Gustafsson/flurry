#include "flurry/hardware/lapic.h"

#include <stdint.h>
#include <flurry/memory/vmm.h>

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800



static uintptr_t hhdm_offset;
static lapic_base;

static inline void get_msr(uint32_t msr, uint32_t *eax, uint32_t *edx) {
    __asm__ volatile ("rdmsr" : "=a"(*eax), "=d"(*edx) : "c"(msr));
}

static inline void set_msr(uint32_t msr, uint32_t eax, uint32_t edx) {
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(eax), "d"(edx));
}

static inline uint32_t lapic_rd(uint16_t reg) {
    return *(volatile uint32_t *) (lapic_base + reg);
}

static inline void lapic_wr(uint16_t reg, uint32_t val) {
    *(volatile uint32_t *) (lapic_base + reg) = val;
}

uintptr_t cpu_get_apic_base() {
    uint32_t eax, edx;
    get_msr(IA32_APIC_BASE_MSR, &eax, &edx);

    uintptr_t phys_addr = (eax & 0xfffff000) | ((edx & 0x0f) << 32);
    vmm_kmap(phys_addr + hhdm_offset, phys_addr, VM_FLAG_WRITE);

    return phys_addr;
}

void cpu_set_apic_base(uintptr_t apic) {
    uint32_t edx = (apic >> 32) & 0x0f;
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;

    set_msr(IA32_APIC_BASE_MSR, eax, edx);
}

void lapic_init(uintptr_t offset) {
    hhdm_offset = offset;
    lapic_base = cpu_get_apic_base() + hhdm_offset;

    cpu_set_apic_base(lapic_base - hhdm_offset);

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    lapic_wr(0xF0, lapic_rd(0xF0) | 0x100);
}
