#include "flurry/interrupts/interrupts.h"

#include "log.h"
#include "flurry/common.h"
#include "flurry/cpu/idt.h"
#include "flurry/hardware/lapic.h"
#include "flurry/log/tty.h"

#define IDT_MAX_DESCRIPTORS 256
#define ISA_IRQ_BASE 0x30
#define PIC_HANDLED_IRQ_COUNT 8
#define PIC1_IRQ_BASE 0x20
#define PIC2_IRQ_BASE 0x28

static const uint8_t FIRST_USABLE_VECTOR = 0x40;
static const uint8_t LAST_USABLE_VECTOR = 0xef;

static uint16_t last_allocated_vector = FIRST_USABLE_VECTOR;
static IntHandler int_handlers[IDT_MAX_DESCRIPTORS];

void unknown_int_handler(InterruptCtx* ctx) {
    kprintf("Unknown interrupt handler, vector: %d\n", ctx->vector);
    kpanic();
}

void pic_irq_handler(InterruptCtx* ctx) {
    kprintf("Unexpected PIC IRQ received, vector: %d\n", ctx->vector);
}

void cpu_exception_handler(InterruptCtx *ctx) {
    kprintf("PANIC: ");
    switch (ctx->vector) {
        case 0:
            kprintf("Divide By Zero\n");
        break;
        case 13:
            unsigned int external = ctx->error_code & 0x1;              // Bit 0
            unsigned int table_indicator = (ctx->error_code >> 2) & 0x1;  // Bit 2 (from the original selector)
            unsigned int selector_index = ctx->error_code >> 3;           // Bits 3 and above

            kprintf("General Protection Fault\n");
            kprintf("Error Code: 0x%x\n", ctx->error_code);
            kprintf("  External: 0x%x\n", external);
            kprintf("  Table Indicator: 0x%x (%s)\n", table_indicator, table_indicator ? "LDT" : "GDT");
            kprintf("  Selector Index: 0x%x\n", selector_index);
            break;

        case 14:
            kprintf("Page Fault\n");
        break;
        default:
            kprintf("IDK");
    }
    kpanic();
}

void interrupt_dispatch(InterruptCtx* ctx) {
    int_handlers[ctx->vector](ctx);
}

uint8_t interrupts_get_isa_irq_vec(uint8_t isa_irq) {
    return ISA_IRQ_BASE + isa_irq;
}

uint8_t interrupts_alloc_vector() {
    kassert(last_allocated_vector == LAST_USABLE_VECTOR, "All usable vectors are exhausted");

    uint8_t ret = last_allocated_vector;
    last_allocated_vector++;
    return ret;
}

void interrupts_set_handler(uint8_t vec, IntHandler handler) {
    int_handlers[vec] = handler;
}

void interrupts_init() {
    for (uint16_t i = 0; i < IDT_MAX_DESCRIPTORS; i++)
        int_handlers[i] = unknown_int_handler;

    for (uint8_t i = 0; i < 32; i++)
        int_handlers[i] = cpu_exception_handler;

    for (uint8_t pic_irq = 0; pic_irq < PIC_HANDLED_IRQ_COUNT; pic_irq++) {
        int_handlers[pic_irq + PIC1_IRQ_BASE] = pic_irq_handler;
        int_handlers[pic_irq + PIC2_IRQ_BASE] = pic_irq_handler;
    }

    logln(LOG_INFO, "[INTERRUPTS] initialized");
}
