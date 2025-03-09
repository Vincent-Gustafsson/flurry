#include "flurry/memory/paging.h"
#include "flurry/memory/pmm.h"



static uintptr_t hhdm_offset;

void paging_set_hhdm_offset(uintptr_t offset) {  hhdm_offset = offset; }

static inline bool is_entry_present(PMLEntry* entry) { return *entry & VM_FLAG_PRESENT; }

static inline void invlpg(uintptr_t addr) { __asm__ volatile("invlpg (%0)" : : "r" (addr) : "memory"); }

static inline void write_cr3(uint64_t cr3) { __asm__ volatile("mov %0, %%cr3" : : "r" (cr3) : "memory"); }

static inline PhysAddr read_cr3() {
    PhysAddr cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r" (cr3) : : "memory");
    return cr3;
}

static PhysAddr get_next_pml(PhysAddr pml, uint16_t pml_index) {
    PMLEntry* table = (PMLEntry*) (pml + hhdm_offset);
    PMLEntry* entry = &table[pml_index];

    if (is_entry_present(entry))
        return *entry & PHYS_ADDR_MASK;

    *entry = ((PMLEntry) pmm_alloc()) | VM_FLAG_PRESENT | VM_FLAG_WRITE | VM_FLAG_USER;
    return *entry & PHYS_ADDR_MASK;
}

static inline PMLEntry* get_pml1_entry(PhysAddr pml4, uintptr_t virt) {
    uint16_t pml4_index = (virt >> 39) & 0x1ff;
    uint16_t pml3_index = (virt >> 30) & 0x1ff;
    uint16_t pml2_index = (virt >> 21) & 0x1ff;
    uint16_t pml1_index = (virt >> 12) & 0x1ff;

    PhysAddr pml3 = get_next_pml(pml4, pml4_index);
    PhysAddr pml2 = get_next_pml(pml3, pml3_index);
    PhysAddr pml1 = get_next_pml(pml2, pml2_index);

    PMLEntry* pml1_hhdm = (PMLEntry*) (pml1 + hhdm_offset);
    PMLEntry* entry = &pml1_hhdm[pml1_index];

    return entry;
}

static inline void invlpg_if_needed(PhysAddr pml4, uintptr_t virt) {
    PhysAddr current_pml4 = read_cr3();
    if (pml4 == current_pml4) {
        invlpg(virt);
    }
}

void paging_load_pml4(PhysAddr pml4) {
    write_cr3(pml4);
}

void paging_map_page(PhysAddr pml4, uintptr_t virt, PhysAddr phys, uint16_t flags) {
    // pages are always mapped with the present flag set
    *get_pml1_entry(pml4, virt) = phys | VM_FLAG_PRESENT | flags;
    invlpg_if_needed(pml4, virt);
}

void paging_unmap_page(PhysAddr pml4, uintptr_t virt) {
    *get_pml1_entry(pml4, virt) = 0;
    invlpg_if_needed(pml4, virt);
}
