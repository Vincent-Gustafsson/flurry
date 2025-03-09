#include <flurry/hardware/lapic.h>

#include "flurry/system/boot.h"
#include "flurry/log/tty.h"

#include "flurry/cpu/gdt.h"
#include "flurry/cpu/idt.h"

#include "flurry/memory/pmm.h"
#include "flurry/memory/vmm.h"
#include "flurry/memory/kmalloc.h"

#include "flurry/acpi/acpi.h"
#include "flurry/acpi/madt.h"
#include "flurry/time/timer.h"



static BootInfo* info;

void kmain(BootInfo* boot_info) {
    info = boot_info;
    tty_init(info->fb->width, info->fb->height, info->fb->address);

    gdt_init();
    idt_init();
    pmm_init(info->hhdm_offset, info->memory_map);
    vmm_init(info->hhdm_offset, info->memory_map, info->kernel_address);
    kmalloc_init();
    acpi_init(info->rsdp_address, info->hhdm_offset);
    timer_init(info->hhdm_offset);

    madt_init();
    lapic_init(info->hhdm_offset);

    /*
    #define SEC_TO_NS(sec) ((sec) * 1000000000ULL)
    #define NS_TO_SEC(ns) ((ns) / 1000000000ULL)

    while (true) {
        kprintf("TIMER TEST :D\n");
        timer_wait_ns(SEC_TO_NS(5));
    }
    */

    for (;;) { __asm__ volatile("hlt"); }
}
