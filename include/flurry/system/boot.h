#pragma once

#include <stdint.h>
#include "flurry/memory/pmm.h"
#include "flurry/graphics/framebuffer.h"

typedef struct {
    uintptr_t virt;
    uintptr_t phys;
} KernelAddress;

typedef struct {
    void* address;
    uint64_t size;
    char* path;
    char* cmdline;
} Module;

typedef struct {
    PhysMemoryMap memory_map;
    uintptr_t hhdm_offset;
    Framebuffer* fb;
    KernelAddress kernel_address;
    uintptr_t rsdp_address;
    Module modules[64];
} BootInfo;
