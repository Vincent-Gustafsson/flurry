#include "flurry/system/boot.h"

#include <flurry/hardware/lapic.h>
//#include "flurry/log/tty.h"

#include <flurry/common.h>
#include <flurry/log/tty.h>

#include "flurry/cpu/gdt.h"
#include "flurry/cpu/idt.h"

#include "flurry/memory/pmm.h"
#include "flurry/memory/vmm.h"
#include "flurry/memory/kmalloc.h"

#include "flurry/acpi/acpi.h"
#include "flurry/acpi/madt.h"
#include "flurry/hardware/ioapic.h"
#include "flurry/interrupts/interrupts.h"
#include "flurry/time/timer.h"
#include "flurry/multitasking/sched.h"

#include "log.h"
#include "flurry/hardware/tsc.h"

void kmain();

static BootInfo* info;

void kinit(BootInfo* boot_info) {
    info = boot_info;
    tty_init(info->fb->width, info->fb->height, info->fb->address);

    gdt_init();
    idt_init();
    interrupts_init();
    pmm_init(info->hhdm_offset, info->memory_map);
    vmm_init(info->hhdm_offset, info->memory_map, info->kernel_address);
    kmalloc_init();
    acpi_init(info->rsdp_address, info->hhdm_offset);
    timer_init(info->hhdm_offset);

    madt_init();
    lapic_init(info->hhdm_offset);

    ioapic_init(info->hhdm_offset);

    tsc_init();

    sched_init();

    for (;;) { __asm__ volatile("hlt"); }
}
