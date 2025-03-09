#include "flurry/acpi/hpet.h"

#include "flurry/common.h"
#include "flurry/memory/vmm.h"
#include "uacpi/acpi.h"
#include "uacpi/tables.h"



typedef struct {
    volatile HpetRegisters* regs;
    uint64_t period;

} HPET;

static uintptr_t hhdm_offset;
static HPET hpet;

uint64_t hpet_get_elapsed_ns() {
    // Convert femtoseconds to nanoseconds.
    return hpet.regs->main_counter * (hpet.period / 1000000);
}

void hpet_reset() {
    hpet.regs->main_counter = 0;
}

static void hpet_setup(uintptr_t addr) {
    // TODO check this
    vmm_kmap(addr + hhdm_offset, addr, VM_FLAG_WRITE | VM_FLAG_NX);

    hpet.regs = (volatile HpetRegisters*) (addr + hhdm_offset);

    hpet.period = hpet.regs->capabilities >> 32;

    hpet.regs->configuration |= 1;
}

void hpet_init(uintptr_t offset) {
    hhdm_offset = offset;

    uacpi_table hpet_table;
    kassert(uacpi_table_find_by_signature(ACPI_HPET_SIGNATURE, &hpet_table) == UACPI_STATUS_OK, "[ACPI] HPET table not found");

    struct acpi_hpet* hpet = hpet_table.ptr;
    hpet_setup(hpet->address.address);

    uacpi_table_unref(&hpet_table);
}
