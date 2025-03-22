#include "flurry/hardware/ioapic.h"

#include <stddef.h>

#include "log.h"
#include "flurry/common.h"
#include "flurry/acpi/madt.h"
#include "flurry/interrupts/interrupts.h"
#include "flurry/log/tty.h"
#include "flurry/memory/vmm.h"

#define ISA_IRQ_COUNT 16



static const uint32_t IOREGSEL = 0x0;
static const uint32_t IOWIN = 0x10;

enum ioapic_regs {
    IOAPICID = 0x0,
    IOAPICVER = 0x1,
    IOAPICARB = 0x2,
    IOREDTBL = 0x10
};

enum ioapic_redtbl_deliv_mode {
    IOAPIC_DELIV_MODE_FIXED     = 0x0,
    IOAPIC_DELIV_MODE_LOW_PRIOR = 0x100,
    IOAPIC_DELIV_MODE_SMI       = 0x200,
    IOAPIC_DELIV_MODE_NMI       = 0x400,
    IOAPIC_DELIV_MODE_INIT      = 0x500,
    IOAPIC_DELIV_MODE_EXTINT    = 0x700
};

enum ioapic_redtbl_dest_mode {
    IOAPIC_DEST_MODE_PHYSICAL = 0x0,
    IOAPIC_DEST_MODE_LOGICAL  = 0x800
};

enum ioapic_deliv_status {
    IOAPIC_DELIV_STATUS_SENT    = 0x0,
    IOAPIC_DELIV_STATUS_PENDING = 0x1000,
};

enum ioapic_redtbl_pin_polarity {
    IOAPIC_ACTIVE_HIGH = 0x0,
    IOAPIC_ACTIVE_LOW  = 0x2000
};

enum ioapic_redtbl_trigger_mode {
    IOAPIC_TRIGGER_EDGE  = 0x0,
    IOAPIC_TRIGGER_LEVEL = 0x8000
};

#define IOAPIC_MASKED 0x10000

static inline uint64_t ioapic_dest(uint8_t lapic_id) {
    return (uint64_t) lapic_id << 56;
}

static uintptr_t hhdm_offset;
static uintptr_t ioapic_addr;

static MadtIoApic* ioapic;

static uint32_t read_reg(uint8_t reg) {
    *(volatile uint32_t*) (ioapic_addr + IOREGSEL + hhdm_offset) = reg;
    return *(volatile uint32_t*) (ioapic_addr + IOWIN + hhdm_offset);
}

static void write_reg(uint8_t reg, uint32_t val) {
    *(volatile uint32_t *) (ioapic_addr + IOREGSEL + hhdm_offset) = reg;
    *(volatile uint32_t *) (ioapic_addr + IOWIN + hhdm_offset) = val;
}

static uint32_t ioapic_get_max_redir_entry() { return (read_reg(IOAPICVER) >> 16) & 0xff; }

static void write_redtbl(uint32_t gsi, uint64_t val) {
    uint32_t relative_gsi = gsi - ioapic->gsi_base;
    uint32_t redir_entry_reg_lo = IOREDTBL + 2 * relative_gsi;
    uint32_t redir_entry_reg_hi = IOREDTBL + 2 * relative_gsi + 1;

    write_reg(redir_entry_reg_lo, val & 0xffffffff);
    write_reg(redir_entry_reg_hi, val >> 32);
}

static MadtIoApicIso* find_iso_by_isa_irq(uint8_t isa_irq) {
    MadtIoApicIso* iso_entries = madt_get_ioapic_isos();

    while (iso_entries != NULL) {
        if (iso_entries->source == isa_irq)
            return iso_entries;

        iso_entries = iso_entries->next;
    }

    return NULL;
}

void ioapic_unmask_isa_irq(uint8_t isa_irq) {
    kassert(isa_irq < ISA_IRQ_COUNT, "idk, isa_irq < 'ISA_IRQ_COUNT'");

    uint8_t cpu_handling_irq_lapic_id = madt_get_lapics()->lapic_id;
    uint8_t isa_irq_vec = interrupts_get_isa_irq_vec(isa_irq);
    MadtIoApicIso* iso = find_iso_by_isa_irq(isa_irq);

    // Map the isa irq according to the ISO.
    if (iso != NULL) {
        // TODO: find ioapic by gsi instead of assuming only one IOAPIC exists.
        // struct ioapic_t *ioapic = madt_find_ioapic_by_gsi(gsi);
        uint64_t redtbl_entry = 0;
        redtbl_entry |= isa_irq_vec;

        if ((iso->flags & MADT_ACTIVE_HIGH) == MADT_ACTIVE_HIGH)
            redtbl_entry |= IOAPIC_ACTIVE_HIGH;
        else if ((iso->flags & MADT_ACTIVE_LOW) == MADT_ACTIVE_LOW)
            redtbl_entry |= IOAPIC_ACTIVE_LOW;

        if ((iso->flags & MADT_TRIGGER_EDGE) == MADT_TRIGGER_EDGE)
            redtbl_entry |= IOAPIC_TRIGGER_EDGE;
        else if ((iso->flags & MADT_TRIGGER_LEVEL) == MADT_TRIGGER_LEVEL)
            redtbl_entry |= IOAPIC_TRIGGER_LEVEL;

        redtbl_entry |= ioapic_dest(cpu_handling_irq_lapic_id);
        write_redtbl(iso->gsi, redtbl_entry);

        return;
    }

    // Identity map the isa irq.
    uint64_t redtbl_entry = 0;
    redtbl_entry |= isa_irq_vec;
    redtbl_entry |= IOAPIC_DELIV_MODE_FIXED;
    redtbl_entry |= IOAPIC_DEST_MODE_PHYSICAL;
    redtbl_entry |= IOAPIC_ACTIVE_HIGH;
    redtbl_entry |= IOAPIC_TRIGGER_EDGE;
    redtbl_entry |= ioapic_dest(cpu_handling_irq_lapic_id);

    write_redtbl(isa_irq, redtbl_entry);
}

void ioapic_init(uintptr_t offset) {
    hhdm_offset = offset;
    ioapic = madt_get_ioapics();

    ioapic_addr = ioapic->ioapic_addr;
    vmm_kmap(ioapic_addr + hhdm_offset, ioapic_addr, VM_FLAG_WRITE | VM_FLAG_NX);

    logln(LOG_INFO, "[IOAPIC] Initialized\n");
}
