#pragma once

#include <stdint.h>

void lapic_init(uintptr_t offset);
void lapic_timer_one_shot(uint64_t ns, uint8_t vec);
void lapic_timer_periodic(uint64_t ns, uint8_t vec);
void lapic_eoi();
void lapic_timer_stop();
