#pragma once

#include <stdint.h>

typedef struct {
    uint64_t capabilities;
    uint64_t _pad0;
    uint64_t configuration;
    uint64_t _pad1;
    uint64_t interrupt_status;
    uint64_t _pad2[0x19];
    uint64_t main_counter;
    uint64_t _pad3;
} __attribute__((packed)) HpetRegisters;

void hpet_init(uintptr_t offset);
uint64_t hpet_get_elapsed_ns();
void hpet_reset();
