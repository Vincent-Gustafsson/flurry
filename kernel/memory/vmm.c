#include <stddef.h>

#include "limine.h"
#include "flurry/memory/vmm.h"

#include "flurry/log/tty.h"
#include "flurry/memory/pmm.h"
#include "flurry/memory/paging.h"
#include "flurry/system/boot.h"

extern uint8_t __KERNEL_START[];
extern uint8_t __TEXT_START[], __TEXT_END[];
extern uint8_t __RODATA_START[], __RODATA_END[];
extern uint8_t __DATA_START[], __DATA_END[];
extern uint8_t __LIMINE_REQUESTS_START[], __LIMINE_REQUESTS_END[];

#define KERNEL_START ((uintptr_t) &__KERNEL_START)
#define TEXT_START ((uintptr_t) &__TEXT_START)
#define TEXT_END ((uintptr_t) &__TEXT_END)
#define RODATA_START ((uintptr_t) &__RODATA_START)
#define RODATA_END ((uintptr_t) &__RODATA_END)
#define DATA_START ((uintptr_t) &__DATA_START)
#define DATA_END ((uintptr_t) &__DATA_END)
// TODO: Gather all desired limine requests in the beginning, put it in a BootInfo struct and call kmain with it.
#define LIMINE_REQ_START ((uintptr_t) &__LIMINE_REQUESTS_START)
#define LIMINE_REQ_END ((uintptr_t) &__LIMINE_REQUESTS_END)



static inline uintptr_t align_up(uintptr_t addr, uintptr_t align)   { return (addr + align - 1) & ~(align - 1); }
static inline uintptr_t align_down(uintptr_t addr, uintptr_t align) { return addr & ~(align - 1); }

static uintptr_t hhdm_offset;

static PhysAddr kernel_pml4;

void vmm_init(uintptr_t offset, PhysMemoryMap memmap, KernelAddress kernel_address) {
    hhdm_offset = offset;
    paging_set_hhdm_offset(offset);

    kernel_pml4 = pmm_alloc();

    // This offset converts a kernel virtual address (as linked) to its corresponding physical address.
    uintptr_t virt_base = kernel_address.virt;
    uintptr_t phys_base = kernel_address.phys;
    uintptr_t kernel_virt_to_phys_offset = virt_base - phys_base;

    // Map kernel TEXT
    for (size_t virt = TEXT_START; virt < TEXT_END; virt += PAGE_SIZE)
        vmm_kmap(virt, virt - kernel_virt_to_phys_offset, VM_FLAG_DEFAULT);

    // Map kernel RODATA
    for (size_t virt = RODATA_START; virt < RODATA_END; virt += PAGE_SIZE)
        vmm_kmap(virt, virt - kernel_virt_to_phys_offset, VM_FLAG_NX);

    // Map kernel DATA
    for (size_t virt = DATA_START; virt < DATA_END; virt += PAGE_SIZE)
        vmm_kmap(virt, virt - kernel_virt_to_phys_offset, VM_FLAG_WRITE | VM_FLAG_NX);

    // TODO: Map limine requests section for now
    for (size_t virt = LIMINE_REQ_START; virt < LIMINE_REQ_END; virt += PAGE_SIZE)
        vmm_kmap(virt, virt - kernel_virt_to_phys_offset, VM_FLAG_NX);

    // Map all physical memory regions with the hhdm_offset
    for (size_t i = 0; i < memmap.entry_count; i++) {
        PhysMemoryRegion entry = memmap.entries[i];

        uintptr_t start = align_down(entry.base, PAGE_SIZE);
        uintptr_t end = align_up(entry.base + entry.length, PAGE_SIZE);
        for (uintptr_t phys = start; phys < end; phys += PAGE_SIZE) {
            vmm_kmap(phys + hhdm_offset, phys, VM_FLAG_WRITE | VM_FLAG_NX);
        }
    }

    vmm_load_pml4(kernel_pml4);
    kprintf("[VMM] Initialized\n");
}

void vmm_load_pml4(PhysAddr pml4) {
    paging_load_pml4(pml4);
}

void vmm_map(PhysAddr pml4, uintptr_t virt, PhysAddr phys, uint64_t flags) {
    paging_map_page(pml4, virt, phys, flags);
}

void vmm_unmap(PhysAddr pml4, uintptr_t virt) {
    paging_unmap_page(pml4, virt);
}

void vmm_kmap(uintptr_t virt, PhysAddr phys, uint64_t flags) {
    paging_map_page(kernel_pml4, virt, phys, flags);
}

void vmm_kunmap(uintptr_t virt) {
    paging_unmap_page(kernel_pml4, virt);
}