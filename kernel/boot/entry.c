#include <stddef.h>

#include "flurry/common.h"
#include "flurry/system/boot.h"
#include "limine.h"

#define LIMINE_REQUEST(request, tag, rev) \
__attribute__((used, section(".limine_requests"))) static volatile struct limine_##request request = { \
.id = tag, \
.revision = rev, \
.response = NULL, \
}



__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);
__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

LIMINE_REQUEST(memmap_request, LIMINE_MEMMAP_REQUEST, 3);
LIMINE_REQUEST(hhdm_request, LIMINE_HHDM_REQUEST, 3);
LIMINE_REQUEST(framebuffer_request, LIMINE_FRAMEBUFFER_REQUEST, 3);
LIMINE_REQUEST(kernel_address_request, LIMINE_KERNEL_ADDRESS_REQUEST, 3);
LIMINE_REQUEST(rsdp_request, LIMINE_RSDP_REQUEST, 3);

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;


extern void kmain(BootInfo* boot_info);
static Framebuffer fb;

void kentry() {
    BootInfo boot_info = {};

    kassert(memmap_request.response, "Unable to get the memory map!");
    kassert(hhdm_request.response, "Unable to get the HHDM response!");
    kassert(framebuffer_request.response, "Unable to get the framebuffer!");
    kassert(kernel_address_request.response, "Unable to get the kernel address info!");
    kassert(rsdp_request.response, "Unable to get the rsdp address!");

    // Get the memory map
    struct limine_memmap_response* mm_res = memmap_request.response;
    PhysMemoryRegion map[mm_res->entry_count];
    for (size_t i = 0; i < mm_res->entry_count; i++) {
        map[i].base = mm_res->entries[i]->base;
        map[i].length = mm_res->entries[i]->length;

        switch (mm_res->entries[i]->type) {
            case LIMINE_MEMMAP_USABLE: map[i].type = PhysMemType_Usable; break;
            case LIMINE_MEMMAP_RESERVED: map[i].type = PhysMemType_RESERVED; break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE: map[i].type = PhysMemType_ACPI_RECLAIMABLE; break;
            case LIMINE_MEMMAP_ACPI_NVS: map[i].type = PhysMemType_ACPI_NVS; break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: map[i].type = PhysMemType_BOOTLOADER_RECLAIMABLE; break;
            case LIMINE_MEMMAP_FRAMEBUFFER: map[i].type = PhysMemType_FRAMEBUFFER; break;
            case LIMINE_MEMMAP_BAD_MEMORY: map[i].type = PhysMemType_BAD_MEMORY; break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES: map[i].type = PhysMemType_KERNEL_AND_MODULES; break;
        }
    }

    // Get the framebuffer
    const struct limine_framebuffer* framebuffer = framebuffer_request.response->framebuffers[0];
    fb.address = framebuffer->address;
    fb.width = framebuffer->width;
    fb.height = framebuffer->height;
    fb.pitch = framebuffer->pitch;
    fb.bpp = framebuffer->bpp;
    fb.memory_model = framebuffer->memory_model;
    fb.red_mask_size = framebuffer->red_mask_size;
    fb.red_mask_shift = framebuffer->red_mask_shift;
    fb.green_mask_size = framebuffer->green_mask_size;
    fb.green_mask_shift = framebuffer->green_mask_shift;
    fb.blue_mask_size = framebuffer->blue_mask_size;
    fb.blue_mask_shift = framebuffer->blue_mask_shift;

    boot_info.memory_map = (PhysMemoryMap) { map, mm_res->entry_count };
    boot_info.hhdm_offset = hhdm_request.response->offset;
    boot_info.fb = &fb;
    boot_info.kernel_address = (KernelAddress) { kernel_address_request.response->virtual_base, kernel_address_request.response->physical_base };
    boot_info.rsdp_address = (uintptr_t) rsdp_request.response->address;

    kmain(&boot_info);
}
