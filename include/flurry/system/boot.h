#pragma once

#include <stdint.h>
#include "flurry/memory/pmm.h"
#include "flurry/graphics/framebuffer.h"

typedef struct {
    uintptr_t virt;
    uintptr_t phys;
} KernelAddress;

typedef struct {
    PhysMemoryMap memory_map;
    uintptr_t hhdm_offset;
    Framebuffer* fb;
    KernelAddress kernel_address;
    uintptr_t rsdp_address;
} BootInfo;
