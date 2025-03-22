#include "flurry/time/timer.h"

#include "log.h"
#include "flurry/log/tty.h"
#include "../../include/flurry/acpi/hpet.h"


static Timer timer;
static uint64_t clock_base_ns = 0;	   // Base counter in nanoseconds.

void timer_init(uintptr_t hhdm_offset) {
    hpet_init(hhdm_offset);
    timer.get_elapsed_ns = hpet_get_elapsed_ns;
    timer.reset = hpet_reset;
    logln(LOG_INFO, "[Timer] Initialized using HPET");
}

uint64_t timer_get_elapsed_ns() {
    return timer.get_elapsed_ns();
}

void timer_set_elapsed_ns(uint64_t value) {
    clock_base_ns = value;
    timer.reset();
}

void timer_wait_ns(uint64_t value) {
    uint64_t time = timer_get_elapsed_ns() + value;
    while (time > timer_get_elapsed_ns()) {
        __asm volatile("pause");
    }
}
