#pragma once

#include <stdint.h>

#include "flurry/system/boot.h"
#include "flurry/memory/pmm.h"
#include "flurry/memory/paging.h"

void vmm_init(uintptr_t offset, PhysMemoryMap memmap, KernelAddress kernel_address);
void vmm_load_pml4(PhysAddr pml4);
void vmm_map(PhysAddr pml4, uintptr_t virt, PhysAddr phys, uint64_t flags);
void vmm_unmap(PhysAddr pml4, uintptr_t virt);
void vmm_kmap(uintptr_t virt, PhysAddr phys, uint64_t flags);
void vmm_kunmap(uintptr_t virt);