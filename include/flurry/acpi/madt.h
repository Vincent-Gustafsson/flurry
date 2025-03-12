#pragma once

#include <stdint.h>

typedef enum {
    MADT_ACTIVE_HIGH = 0x1,
    MADT_ACTIVE_LOW = 0x3
} MadtPinPolarity;

typedef enum {
    MADT_TRIGGER_EDGE = 0x4,
    MADT_TRIGGER_LEVEL = 0xc
} MadtTriggerMode;

typedef struct MadtLApic {
    uint8_t acpi_id;
    uint8_t lapic_id;
    uint8_t flags;
    struct MadtLApic* next;
} __attribute__((packed)) MadtLApic;

typedef struct MadtLapicNmiEntry {
    uint8_t uid;
    uint16_t flags;
    uint8_t lint;
    struct MadtLapicNmiEntry* next;
} MadtLApicNmi;

typedef struct MadtIoApic {
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_addr;
    uint32_t gsi_base;
    struct MadtIoApic* next;
} __attribute__((packed)) MadtIoApic;

typedef struct MadtIoApicIso {
    uint8_t bus;
    uint8_t source;
    uint32_t gsi;
    uint16_t flags;
    struct MadtIoApicIso* next;
} MadtIoApicIso;

typedef struct MadtIoApicNmi {
    uint16_t flags;
    uint32_t gsi;
    struct MadtIoApicNmi* next;
} MadtIoApicNmi;

void madt_init();

MadtLApic* madt_get_lapics();
MadtLApicNmi* madt_get_lapic_nmis();
MadtIoApic* madt_get_ioapics();
MadtIoApicIso* madt_get_ioapic_isos();
MadtIoApicNmi* madt_get_ioapic_nmis();
