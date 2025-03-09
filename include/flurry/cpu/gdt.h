#pragma once
#include <stdint.h>

typedef uint64_t gdt_entry;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdtr_t;

typedef struct {
    uint32_t reserved1;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved3;
    uint16_t reserved4;
    uint16_t iopb;
} __attribute__((packed)) TSS;

extern void set_gdt(uint16_t limit, uint64_t base);
extern void set_tss();
extern void reload_segments();
void gdt_init(void);
