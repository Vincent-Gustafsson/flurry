#include "flurry/cpu/gdt.h"
#include <stdint.h>

#include "log.h"
#include "flurry/log/tty.h"

#define ENABLE_INTERRUPTS() asm volatile ("sti")
#define DISABLE_INTERRUPTS() asm volatile ("cli")

static uint64_t gdt[7];
static TSS tss;

void gdt_encode_entry(uint64_t *entry, uint64_t limit, uint64_t base, uint64_t access, uint64_t flags)
{
    *entry = (limit & 0xffff) | ((base & 0xffff) << 16) | ((base & 0xff0000) << 32) | ((access & 0xff) << 40) | (((limit & 0xf0000) >> 16) << 48) | ((flags & 0xf) << 52) | (((base & 0xff000000) >> 24) << 56);
}

void gdt_init(void)
{
    DISABLE_INTERRUPTS();
    // Null Descriptor
    gdt[0] = 0x0;
    // kernel code
    gdt_encode_entry(&gdt[1], 0xfffff, 0x0, 0x9a, 0xa);
    // kernel data
    gdt_encode_entry(&gdt[2], 0xfffff, 0x0, 0x92, 0xc);
    // user code
    gdt_encode_entry(&gdt[3], 0xfffff, 0x0, 0xfa, 0xa);
    // user data
    gdt_encode_entry(&gdt[4], 0xfffff, 0x0, 0xf2, 0xc);
    // TSS
    gdt_encode_entry(&gdt[5], sizeof(TSS), ((uint64_t) &tss) & 0xffffffff, 0x89, 0x40);
    gdt[6] = (((uint64_t) &tss) >> 32) & 0xffffffff;
    tss.iopb = sizeof(gdt);

    set_gdt((sizeof(uint64_t) * 7) - 1, gdt);
    set_tss();
    reload_segments();
    ENABLE_INTERRUPTS();

    logln(LOG_INFO, "[GDT] Initialized");
}
