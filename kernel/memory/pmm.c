#include <stddef.h>
#include <string.h>

#include "flurry/memory/pmm.h"
#include "flurry//log/tty.h"



static uint64_t hhdm_offset;
FreePage* free_list_head = NULL;

void pmm_init(uintptr_t offset, PhysMemoryMap memory_map) {
    hhdm_offset = offset;

    uint64_t num_of_page_frames = 0;
    uint64_t total_mem = 0;
    for (size_t i = 1; i < memory_map.entry_count; i++) {
        PhysMemoryRegion entry = memory_map.entries[i];

        // Filter out usable memory regions
        if (entry.type != PhysMemType_Usable)
            continue;

        total_mem += entry.length;

        // Iterate through the region in page-sized chunks
        for (uint64_t addr = entry.base; addr < entry.base + entry.length; addr += PAGE_SIZE) {
            // Create a new page frame and add it to the list
            FreePage *page = (FreePage*) (addr + hhdm_offset);
            page->address = addr;
            page->next = free_list_head;
            free_list_head = page;
            num_of_page_frames++;
        }
    }

    kprintf("[PMM] Initialized with %d page frames\n", num_of_page_frames);
}



PhysAddr pmm_alloc() {
    if (free_list_head == NULL)
        return NULL;

    FreePage* page = free_list_head;
    free_list_head = page->next;

    // Save the address before it gets removed by memset.
    PhysAddr addr = page->address;
    memset((void*) page->address + hhdm_offset, 0, PAGE_SIZE);

    return addr;
}

void pmm_free(PhysAddr addr) {
    FreePage* page = (FreePage*) addr;
    page->next = free_list_head;
    free_list_head = page;
}
