#pragma once

#include <stdint.h>

void tsc_init();

uint64_t tsc_read();
uint64_t tsc_read_ns();
uint64_t tsc_ns_to_ticks(uint64_t time);
uint64_t tsc_ticks_to_ns(uint64_t ticks);
