#pragma once

#include <stdint.h>
#include <stdbool.h>
#define PAGE_SIZE 4096  // 4 KB

typedef uint64_t PhysAddr;

typedef enum {
    PhysMemType_Usable,
    PhysMemType_RESERVED,
    PhysMemType_ACPI_RECLAIMABLE,
    PhysMemType_ACPI_NVS,
    PhysMemType_BAD_MEMORY,
    PhysMemType_BOOTLOADER_RECLAIMABLE,
    PhysMemType_KERNEL_AND_MODULES,
    PhysMemType_FRAMEBUFFER,
} PhysMemoryRegionType;

typedef struct {
    PhysAddr base;
    uint64_t length;
    PhysMemoryRegionType type;
} PhysMemoryRegion;

typedef struct {
    PhysMemoryRegion* entries;
    uint64_t entry_count;
} PhysMemoryMap;

typedef struct FreePage {
    PhysAddr address;
    struct FreePage* next;
} FreePage;

void pmm_init(uintptr_t offset, PhysMemoryMap memory_map);
PhysAddr pmm_alloc();
void pmm_free(PhysAddr addr);
