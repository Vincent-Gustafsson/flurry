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

uint64_t testt;

void test(InterruptCtx* _) {
    logln(LOG_DEBUG, "Time: %llu ns; Diff: %llu ns", tsc_read_ns() - testt, tsc_read_ns() - testt - from_ms(50));
    lapic_eoi();
}

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

    interrupts_set_handler(32, test);
    testt = tsc_read_ns();
    logln(LOG_DEBUG, "Before: %llu ns", testt);
    lapic_timer_one_shot(from_ms(50), 32);


    /*


    uint64_t now = tsc_read_ns();
    logln(LOG_DEBUG, "Before: %llu ns, ticks: %llu", now);
    lapic_timer_one_shot(from_ms(50), 32);
    //lapic_timer_tsc_deadline(now + from_ms(5000), 32);

    //sched_init();

    */
    for (;;) { __asm__ volatile("hlt"); }
}
