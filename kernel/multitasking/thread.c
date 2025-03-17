#include "flurry/multitasking/thread.h"

#include <stdbool.h>

#include "flurry/string.h"
#include "flurry/memory/kmalloc.h"

#define STACK_SIZE 4096 * 4

#define UTHREAD true
#define KTHREAD false

#define KERNEL_CS 0x08
#define KERNEL_SS 0x10 // Actually the DATA SEGMENT


_Atomic uint64_t tid_counter = 0;

/*InterruptCtx new_ctx(uintptr_t entry, uintptr_t stack, bool is_uthread) {
    return {
        .rip = entry,
        .rsp = stack,
        .cs = is_uthread ? 0x69 : KERNEL_CS,
        .ss = is_uthread ? 0x69 : KERNEL_SS,
        .rbp = 0,
        .rflags = 0x202
    };
}*/

Thread* thread_kcreate(char* name, Process* proc, void (*entry)(void)) {
    Thread* t = kmalloc(sizeof(Thread));

    strcpy(t->name, name);
    t->tid = tid_counter++;
    t->proc = proc;
    t->status = THREAD_READY;

    uintptr_t k_stack = (uintptr_t) (kmalloc(STACK_SIZE) + STACK_SIZE);
    t->k_base = k_stack;
    t->k_rsp = k_stack;

    //t->ctx = new_ctx((uintptr_t) entry, k_stack, KTHREAD);
    t->rsp = k_stack;

    return t;
}

