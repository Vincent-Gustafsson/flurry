#pragma once
#include <stdint.h>

void ioapic_init(uintptr_t offset);
void ioapic_unmask_isa_irq(uint8_t isa_irq);
