#pragma once

#include <stdint.h>

#define PAGE_SIZE         4096
#define PML_ENTRY_COUNT   512

#define VM_FLAG_PRESENT   (1ULL << 0)
#define VM_FLAG_WRITE     (1ULL << 1)
#define VM_FLAG_USER      (1ULL << 2)
#define VM_FLAG_NO_CACHE     (1ULL << 4)
#define VM_FLAG_NX        (1ULL << 63)
#define VM_FLAG_DEFAULT   (0ULL)

// Mask for extracting the physical address (bits 12–51).
#define PHYS_ADDR_MASK    0x000FFFFFFFFFF000ULL

typedef uint64_t PhysAddr;

typedef uint64_t PMLEntry;

typedef struct {
    PMLEntry entries[PML_ENTRY_COUNT];
} PML4, PML3, PML2, PML1;


void paging_set_hhdm_offset(uintptr_t offset);
void paging_load_pml4(PhysAddr pml4);
void paging_map_page(PhysAddr pml4, uintptr_t virt, PhysAddr phys, uint16_t flags);
void paging_unmap_page(PhysAddr pml4, uintptr_t virt);
