#include "flurry/multitasking/sched.h"

#include <flurry/log/tty.h>

#include "flurry/hardware/lapic.h"
#include "flurry/memory/vmm.h"
#include "flurry/multitasking/process.h"
#include "flurry/multitasking/thread.h"



Process* kernel_process;
Thread* kernel_threads;

static void schedule() {
    kprintf("LEZZGOO: %s", kernel_threads->name);
}

static void handle_lapic_timer_int(InterruptCtx* ctx) {
    lapic_eoi();
    schedule();
}

void sched_init(void (*entry)(void)) {
    kernel_process = process_create("kernel", vmm_get_kernel_pml4());
    Thread* kmain_thread = thread_kcreate("kmain", kernel_process, entry);

    kernel_threads = kmain_thread;

    interrupts_set_handler(32, handle_lapic_timer_int);
    lapic_timer_periodic(1000000000, 32);
}
