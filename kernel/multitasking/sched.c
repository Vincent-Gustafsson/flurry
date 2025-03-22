#include "flurry/multitasking/sched.h"

#include <stddef.h>
#include <flurry/log/tty.h>
#include <sys/types.h>

#include "flurry/string.h"
#include "flurry/hardware/lapic.h"
#include "flurry/memory/kmalloc.h"
#include "flurry/memory/vmm.h"
#include "flurry/multitasking/process.h"
#include "flurry/multitasking/thread.h"
#include "flurry/time/timer.h"



Thread* switch_context(Thread* old, Thread* new);

Thread* current_thread;

void sched() {
    Thread* old = current_thread;
    Thread* next = old->next;
    current_thread = next;

    Thread* new = switch_context(old, next);

    old->status = THREAD_READY;
    new->status = THREAD_RUNNING;
}

void handle_timer_int(InterruptCtx* ctx) {
    lapic_eoi();
    sched();
}

void one() {
    while (true) {
        kprintf("hello from 1\n");
        timer_wait_ns(1000000000 / 2);
    }
}

void two() {
    while (true) {
        kprintf("hello from 2\n");
        timer_wait_ns(1000000000 / 2);
    }
}

void sched_start() {
    interrupts_set_handler(32, handle_timer_int);
    lapic_timer_periodic(5000000, 32);
    sched();
}

void sched_init() {
    Thread* init_thread = kmalloc(sizeof(Thread));

    Thread* t_one = thread_kcreate("one", one, NULL);
    Thread* t_two = thread_kcreate("two", two, NULL);

    t_one->next = t_two;
    t_two->next = t_one;

    init_thread->next = t_one;
    current_thread = init_thread;

    sched_start();
}
