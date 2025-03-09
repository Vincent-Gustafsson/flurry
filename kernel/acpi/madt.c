#include "flurry/acpi/madt.h"

#include <uacpi/acpi.h>
#include <uacpi/tables.h>

#include "flurry/common.h"
#include "flurry/memory/kmalloc.h"



MadtLApic* lapic_list[8];
uint8_t lapic_count = 0;

MadtIoApic* ioapic_list[8];
uint8_t ioapic_count = 0;

static void get_lapic_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_lapic* lapic_table_entry = (struct acpi_madt_lapic*) entry;
    MadtLApic* lapic_entry = kmalloc(sizeof(MadtLApic));

    lapic_entry->acpi_id = lapic_table_entry->uid;
    lapic_entry->lapic_id = lapic_table_entry->id;
    lapic_entry->flags = lapic_table_entry->flags;

    lapic_list[lapic_count++] = lapic_entry;
}

static void get_ioapic_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_ioapic* ioapic_table_entry = (struct acpi_madt_lapic*) entry;
    MadtIoApic* ioapic_entry = kmalloc(sizeof(MadtIoApic));

    ioapic_entry->ioapic_id = ioapic_table_entry->id;
    ioapic_entry->reserved = ioapic_table_entry->rsvd;
    ioapic_entry->ioapic_addr = ioapic_table_entry->address;
    ioapic_entry->gsi_base = ioapic_table_entry->gsi_base;

    ioapic_list[ioapic_count++] = ioapic_entry;
}

void madt_init() {
    uacpi_table madt_table;
    kassert(uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &madt_table) == UACPI_STATUS_OK, "[ACPI] MADT table not found\n");

    struct acpi_madt* madt = madt_table.ptr;

    uintptr_t lapic_addr = madt->local_interrupt_controller_address;

    size_t offset = 0;
    while (offset < madt->hdr.length - sizeof(*madt)) {
        struct acpi_entry_hdr* entry = (struct acpi_entry_hdr*) (((uintptr_t) madt->entries) + offset);

        switch (entry->type) {
            case ACPI_MADT_ENTRY_TYPE_LAPIC:  get_lapic_entry(entry); break;
            case ACPI_MADT_ENTRY_TYPE_IOAPIC: get_ioapic_entry(entry); break;
            default: break;
        }

        offset += entry->length;
    }

    uacpi_table_unref(&madt_table);
}
