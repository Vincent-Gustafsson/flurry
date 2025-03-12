#pragma once

#include <stdint.h>

static inline void read_msr(uint32_t msr, uint32_t *eax, uint32_t *edx) {
    __asm__ volatile ("rdmsr" : "=a"(*eax), "=d"(*edx) : "c"(msr));
}

static inline void write_msr(uint32_t msr, uint32_t eax, uint32_t edx) {
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(eax), "d"(edx));
}

static inline void write_portb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t read_portb(uint16_t port) {
    uint8_t value;
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}
