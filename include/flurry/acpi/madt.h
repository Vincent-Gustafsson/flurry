#pragma once

#include <stdint.h>

typedef struct {
    uint8_t acpi_id;
    uint8_t lapic_id;
    uint8_t flags;
} __attribute__((packed)) MadtLApic;

typedef struct {
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
} __attribute__((packed)) MadtIoApic;



void madt_init();
