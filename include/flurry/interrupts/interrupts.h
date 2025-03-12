#pragma once
#include <stdint.h>

typedef struct {
    uint64_t cr4, cr3, cr2, cr0;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rsi, rdi, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error_code, rip, cs, rflags, rsp, ss;
} __attribute__((packed)) InterruptCtx;

typedef void (*IntHandler)(InterruptCtx *ctx);

void interrupts_init();
uint8_t interrupts_get_isa_irq_vec(uint8_t isa_irq);
uint8_t interrupts_alloc_vector();
void interrupts_set_handler(uint8_t vec, IntHandler handler);

