#include "flurry/hardware/tsc.h"

#include <stdint.h>
#include <flurry/time/timer.h>

#include "x86gprintrin.h"
#include "log.h"

#define ONE_SECOND 1000000000ULL


uint64_t tsc_freq;

uint64_t tsc_read() { return __rdtsc(); }

uint64_t tsc_read_ns() { return tsc_ticks_to_ns(tsc_read()); }

uint64_t tsc_ns_to_ticks(uint64_t time) { return tsc_freq * time / ONE_SECOND; }

uint64_t tsc_ticks_to_ns(uint64_t ticks) {
    uint64_t quot = ticks / tsc_freq;
    uint64_t rem = ticks % tsc_freq;
    return quot * ONE_SECOND + (rem * ONE_SECOND) / tsc_freq;
}

void tsc_init() {
    uint64_t ten_ms = 10000000;

    uint64_t tsc_start = tsc_read();
    timer_wait_ns(ten_ms);
    uint64_t tsc_end = tsc_read();

    // 10 ms * 100 = 1 s, tsc_freq in Hz
    tsc_freq = (tsc_end - tsc_start) * 100;

    logln(LOG_INFO, "[TSC] %u MHz", tsc_freq / 1000000ULL);
}
