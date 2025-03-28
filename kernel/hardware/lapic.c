#include "flurry/hardware/lapic.h"

#include <stddef.h>
#include <stdint.h>
#include <flurry/common.h>

#include "flurry/asm_wrappers.h"
#include <flurry/memory/vmm.h>

#include "log.h"
#include "flurry/acpi/madt.h"
#include "flurry/hardware/tsc.h"
#include "flurry/interrupts/interrupts.h"
#include "flurry/log/tty.h"
#include "flurry/time/timer.h"

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define LAPIC_SOFTWARE_ENABLE (1 << 8)
#define LAPIC_SPURIOUS_VECTOR 0xF0
#define LAPIC_NMI_ALL_CPUS 0xFF

#define PIC1_IRQ_BASE 0x20
#define PIC2_IRQ_BASE 0x28



typedef enum {
    LAPIC_DELIV_MODE_FIXED  = 0x0,
    LAPIC_DELIV_MODE_SMI    = 0x200,
    LAPIC_DELIV_MODE_NMI    = 0x400,
    LAPIC_DELIV_MODE_INIT   = 0x500,
    LAPIC_DELIV_MODE_EXTINT = 0x700
} LapicDeliveryMode;

typedef enum {
    LVT_TIMER_ONE_SHOT     = 0x0,
    LVT_TIMER_PERIODIC     = 0x20000,
    LVT_TIMER_TSC_DEADLINE = 0x40000
} LapicLvtTimerMode;

typedef enum {
    REG_LAPIC_ID = 0x20,
    REG_LAPIC_VER = 0x30,
    REG_TPR = 0x80,
    REG_EOI = 0xb0,
    REG_SPURIOUS = 0xf0,
    REG_ICR_LOW = 0x300,
    REG_ICR_HIGH = 0x310,
    REG_LVT_TIMER = 0x320,
    REG_LVT_LINT0 = 0x350,
    REG_LVT_LINT1 = 0x360,
    REG_TIMER_INIT_COUNT = 0x380,
    REG_TIMER_CURR_COUNT = 0x390,
    REG_TIMER_DIV = 0x3e0
} LapicReg;

static uintptr_t hhdm_offset;

static uintptr_t lapic_phys_base;
static uintptr_t lapic_base;

static uint64_t lapic_calibration_ticks; // TODO, move to CPU struct

static const uint16_t PIC1_COMMAND_PORT = 0x20;
static const uint16_t PIC1_DATA_PORT = 0x21;
static const uint16_t PIC2_COMMAND_PORT = 0xa0;
static const uint16_t PIC2_DATA_PORT = 0xa1;

// Slave PIC was connected to the master PIC via IRQ 2 on the master PIC
static const uint8_t IRQ_SLAVE_PIC_TO_MASTER_PIC = 2;

static const uint8_t ICW1_ICW4_PRESENT = 0x01;
static const uint8_t ICW1_INIT = 0x10;
static const uint8_t ICW4_8086 = 0x01;

void disable_pic(void) {
    // ICW1: Start initialization in cascade mode
    write_portb(PIC1_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_PRESENT);
    write_portb(PIC2_COMMAND_PORT, ICW1_INIT | ICW1_ICW4_PRESENT);

    // ICW2: Set vector offset (remap IRQ's)
    write_portb(PIC1_DATA_PORT, PIC1_IRQ_BASE);
    write_portb(PIC2_DATA_PORT, PIC2_IRQ_BASE);

    // ICW3: Slave PIC is connected to master PIC via IRQ 2
    write_portb(PIC1_DATA_PORT, 1 << IRQ_SLAVE_PIC_TO_MASTER_PIC);
    write_portb(PIC2_DATA_PORT, IRQ_SLAVE_PIC_TO_MASTER_PIC);

    // ICW4: Use 8086 mode
    write_portb(PIC1_DATA_PORT, ICW4_8086);
    write_portb(PIC2_DATA_PORT, ICW4_8086);

    // Mask all IRQ's
    write_portb(PIC1_DATA_PORT, 0xff);
    write_portb(PIC2_DATA_PORT, 0xff);
}

static inline uint32_t read_reg(LapicReg reg) {
    return *((volatile uint32_t *) (lapic_base + reg));
}

static inline void write_reg(LapicReg reg, uint32_t val) {
    *((volatile uint32_t *) (lapic_base + reg)) = val;
}

static uintptr_t cpu_get_apic_base() {
    uint32_t eax, edx;
    read_msr(IA32_APIC_BASE_MSR, &eax, &edx);

    uintptr_t phys_addr = (eax & 0xfffff000) | ((edx & 0x0f) << 32);
    return phys_addr;
}

static void cpu_set_apic_base(uintptr_t apic) {
    uint32_t edx = (apic >> 32) & 0x0f;
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;

    write_msr(IA32_APIC_BASE_MSR, eax, edx);
}

static void lapic_spurious_handler(struct int_ctx_t *ctx) {
    kprintf("[LAPIC] spurious interrupt\n");
}

static void configure_nmis(MadtLApicNmi* lapic_nmis) {
    MadtLApicNmi* lapic_nmi = lapic_nmis;

    while (lapic_nmi != NULL) {
        // Once MP is setup get from proper cpu struct.
        if (lapic_nmi->uid != LAPIC_NMI_ALL_CPUS && lapic_nmi->uid != madt_get_lapics()->acpi_id) {
            lapic_nmi = lapic_nmi->next;
            continue;
        }

        uint16_t lvt_flags = 0;
        if ((lapic_nmi->flags & MADT_ACTIVE_HIGH) == MADT_ACTIVE_HIGH)
            lvt_flags = MADT_ACTIVE_HIGH;
        else if ((lapic_nmi->flags & MADT_ACTIVE_LOW) == MADT_ACTIVE_LOW)
            lvt_flags = MADT_ACTIVE_LOW;

        // "trigger mode is always edge sensitive for NMI delivery mode" - https://github.com/Aster-OS/aster/blob/main/kernel/src/arch/x86_64/apic/lapic.c

        // "100b if NMI" (0x4) - https://wiki.osdev.org/APIC#Local_Vector_Table_Registers
        if (lapic_nmi->lint == 0)
            write_reg(REG_LVT_LINT0, lvt_flags | LAPIC_DELIV_MODE_NMI | 0x4);
        else if (lapic_nmi->lint == 1)
            write_reg(REG_LVT_LINT1, lvt_flags | LAPIC_DELIV_MODE_NMI | 0x4);

        lapic_nmi = lapic_nmi->next;
    }
}

void lapic_timer_stop() {
    write_reg(REG_TIMER_INIT_COUNT, 0);
}

static void calibrate_timer() {
    // Set the LAPIC divider to 16.
    // On many systems, writing 0x3 to REG_TIMER_DIV corresponds to divide-by-16.
    write_reg(REG_TIMER_DIV, 0x3);

    // Mask the LAPIC timer interrupt during calibration.
    //write_reg(REG_LVT_TIMER, 0x10000);

    // We'll use a 10ms calibration interval.
    const uint64_t calibration_interval_ns = 10000000ULL;
    const int iterations = 5;
    uint64_t total_ticks = 0;

    for (int i = 0; i < iterations; i++) {
        // Load the timer with the maximum count.
        uint32_t init_count = UINT32_MAX;
        write_reg(REG_TIMER_INIT_COUNT, init_count);

        // Wait for the calibration interval.
        timer_wait_ns(calibration_interval_ns);

        // Read the current count.
        uint32_t current_count = read_reg(REG_TIMER_CURR_COUNT);

        // Calculate the number of ticks elapsed in this iteration.
        uint32_t ticks_elapsed = init_count - current_count;
        total_ticks += ticks_elapsed;
    }

    // Average the calibration ticks.
    lapic_calibration_ticks = total_ticks / iterations;

    logln(LOG_DEBUG, "[LAPIC] Timer calibrated: %lu ticks in %lu ns (averaged over %d iterations)",
          lapic_calibration_ticks, calibration_interval_ns, iterations);
}

uint64_t ns_to_lapic_ticks(uint64_t ns) {
    // Since lapic_calibration_ticks is measured over 10ms (10,000,000 ns),
    // we can scale it:
    //    ticks = ns * (lapic_calibration_ticks / 10,000,000)
    return (ns * lapic_calibration_ticks) / 10000000ULL;
}

void lapic_eoi() {
    write_reg(REG_EOI, 0);
}

void lapic_timer_one_shot(uint64_t ns, uint8_t vec) {
    logln(LOG_DEBUG, "before one: %llu ns", tsc_read_ns());
    uint32_t ticks = clamp(ns_to_lapic_ticks(ns), 1, UINT32_MAX);
    write_reg(REG_LVT_TIMER, LVT_TIMER_ONE_SHOT | vec);
    write_reg(REG_TIMER_INIT_COUNT, ticks);
    logln(LOG_DEBUG, "after one: %llu ns", tsc_read_ns());
}

void lapic_timer_periodic(uint64_t ns, uint8_t vec) {
    uint32_t ticks = clamp(ns_to_lapic_ticks(ns), 1, UINT32_MAX);
    write_reg(REG_LVT_TIMER, LVT_TIMER_PERIODIC | vec);
    write_reg(REG_TIMER_INIT_COUNT, ticks);
}

void lapic_timer_tsc_deadline(uint64_t deadline, uint8_t vec) {
    uint64_t delta = deadline - tsc_read_ns();
    logln(LOG_DEBUG, "tsc_deadline delta: %lu ns", delta);
    lapic_timer_one_shot(delta, vec);
}

void lapic_init(uintptr_t offset) {
    hhdm_offset = offset;

    disable_pic();

    // Get and set the lapic address
    lapic_phys_base = cpu_get_apic_base();
    lapic_base = lapic_phys_base + hhdm_offset;
    vmm_kmap(lapic_base, lapic_phys_base, VM_FLAG_WRITE | VM_FLAG_NO_CACHE);
    cpu_set_apic_base(lapic_phys_base);

    // Configure the spurious interrupt vector and software enable (1 << 8)
    interrupts_set_handler(LAPIC_SPURIOUS_VECTOR, lapic_spurious_handler);
    lapic_timer_stop();
    write_reg(REG_SPURIOUS, LAPIC_SOFTWARE_ENABLE | LAPIC_SPURIOUS_VECTOR);

    // Configure NMI sources
    configure_nmis(madt_get_lapic_nmis());

    calibrate_timer();

    logln(LOG_DEBUG, "[LAPIC] Timer calibrated, %lu ticks in 10 ms", lapic_calibration_ticks);
    logln(LOG_INFO, "[LAPIC] Initialized");
}
