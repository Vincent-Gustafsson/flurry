#pragma once

#include <stdint.h>

typedef struct {
    // Returns how many nanoseconds have elapsed since initialization.
    uint64_t (*get_elapsed_ns)();

    // Resets the clock's counter.
    void (*reset)();
} Timer;

void timer_init(uintptr_t hhdm_offset);

uint64_t timer_get_elapsed_ns();
void timer_set_elapsed_ns(uint64_t value);
void timer_wait_ns(uint64_t value);
