#include "flurry/acpi/madt.h"

#include <uacpi/acpi.h>
#include <uacpi/tables.h>

#include "flurry/common.h"
#include "flurry/memory/kmalloc.h"



static MadtLApic* lapics = NULL;
static MadtLApicNmi* lapic_nmis = NULL;
static MadtIoApic* ioapics = NULL;
static MadtIoApicNmi* ioapic_nmis = NULL;

static MadtIoApicIso* ioapic_isos = NULL;

MadtLApic* madt_get_lapics() { return lapics; }
MadtLApicNmi* madt_get_lapic_nmis() { return lapic_nmis; }
MadtIoApic* madt_get_ioapics() { return ioapics; }
MadtIoApicIso* madt_get_ioapic_isos() { return ioapic_isos; }
MadtIoApicNmi* madt_get_ioapic_nmis() { return ioapic_nmis; }

static void get_lapic_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_lapic* lapic_table_entry = (struct acpi_madt_lapic*) entry;
    MadtLApic* lapic_entry = kmalloc(sizeof(MadtLApic));

    lapic_entry->acpi_id = lapic_table_entry->uid;
    lapic_entry->lapic_id = lapic_table_entry->id;
    lapic_entry->flags = lapic_table_entry->flags;
    lapic_entry->next = lapics;

    lapics = lapic_entry;
}

static void get_ioapic_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_ioapic* ioapic_table_entry = (struct acpi_madt_ioapic*) entry;
    MadtIoApic* ioapic_entry = kmalloc(sizeof(MadtIoApic));

    ioapic_entry->ioapic_id = ioapic_table_entry->id;
    ioapic_entry->reserved = ioapic_table_entry->rsvd;
    ioapic_entry->ioapic_addr = ioapic_table_entry->address;
    ioapic_entry->gsi_base = ioapic_table_entry->gsi_base;
    ioapic_entry->next = ioapics;

    ioapics = ioapic_entry;
}

static void get_lapic_nmi_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_lapic_nmi* nmi_table_entry = (struct acpi_madt_lapic_nmi*) entry;
    MadtLApicNmi* nmi_entry = kmalloc(sizeof(MadtLApicNmi));

    nmi_entry->uid = nmi_table_entry->uid;
    nmi_entry->flags = nmi_table_entry->flags;
    nmi_entry->lint = nmi_table_entry->lint;
    nmi_entry->next = lapic_nmis;

    lapic_nmis = nmi_entry;
}

void get_ioapic_iso_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_interrupt_source_override* iso_table_entry = (struct acpi_madt_interrupt_source_override*) entry;
    MadtIoApicIso* iso_entry = kmalloc(sizeof(MadtIoApicIso));

    iso_entry->bus = iso_table_entry->bus;
    iso_entry->source = iso_table_entry->source;
    iso_entry->gsi = iso_table_entry->gsi;
    iso_entry->flags = iso_table_entry->flags;
    iso_entry->next = ioapic_isos;

    ioapic_isos = iso_entry;
}

void get_ioapic_nmi_entry(struct acpi_entry_hdr* entry) {
    struct acpi_madt_nmi_source* nmi_table_entry = (struct acpi_madt_nmi_source*) entry;
    MadtIoApicNmi* nmi_entry = kmalloc(sizeof(MadtIoApicNmi));

    nmi_entry->flags = nmi_table_entry->flags;
    nmi_entry->gsi = nmi_table_entry->gsi;
    nmi_entry->next = ioapic_nmis;

    ioapic_nmis = nmi_entry;
}

void madt_init() {
    uacpi_table madt_table;
    kassert(uacpi_table_find_by_signature(ACPI_MADT_SIGNATURE, &madt_table) == UACPI_STATUS_OK, "[ACPI] MADT table not found\n");

    struct acpi_madt* madt = madt_table.ptr;

    size_t offset = 0;
    while (offset < madt->hdr.length - sizeof(*madt)) {
        struct acpi_entry_hdr* entry = (struct acpi_entry_hdr*) (((uintptr_t) madt->entries) + offset);

        switch (entry->type) {
            case ACPI_MADT_ENTRY_TYPE_LAPIC:  get_lapic_entry(entry); break;
            case ACPI_MADT_ENTRY_TYPE_LAPIC_NMI: get_lapic_nmi_entry(entry); break;
            case ACPI_MADT_ENTRY_TYPE_IOAPIC: get_ioapic_entry(entry); break;
            case ACPI_MADT_ENTRY_TYPE_INTERRUPT_SOURCE_OVERRIDE: get_ioapic_iso_entry(entry); break;
            case ACPI_MADT_ENTRY_TYPE_NMI_SOURCE: get_ioapic_nmi_entry(entry); break;
            default: break;
        }

        offset += entry->length;
    }

    uacpi_table_unref(&madt_table);
}
