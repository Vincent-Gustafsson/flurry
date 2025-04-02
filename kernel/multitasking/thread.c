#include "flurry/multitasking/thread.h"

#include <stdbool.h>
#include <uacpi/internal/resources.h>

#include "flurry/common.h"
#include "flurry/string.h"
#include "flurry/log/tty.h"
#include "flurry/memory/kmalloc.h"
#include "flurry/multitasking/events.h"
#include "flurry/multitasking/sched.h"

#define STACK_SIZE 4096 * 4

#define UTHREAD true
#define KTHREAD false

#define KERNEL_CS 0x08
#define KERNEL_SS 0x10 // Actually the DATA SEGMENT

#define PUSH_ON_STACK(stack, type, item) *((type *)(stack -= sizeof(type))) = item



_Atomic uint64_t tid_counter = 0;


Thread* thread_kcreate(char* name, void (*entry)(void), Process* proc) {
    Thread* t = kmalloc(sizeof(Thread));

    strcpy(t->name, name);
    t->tid = tid_counter++;
    t->process = proc;

    t->rsp_base = kmalloc(STACK_SIZE);

    InitialKStack* stack = (InitialKStack*) (t->rsp_base + STACK_SIZE - sizeof(InitialKStack));
    stack->rflags = 0x202;
    stack->init_thread = init_thread;
    stack->entry = entry;

    t->rsp = (uintptr_t) stack;

    t->event = event_create();
    t->event->owner = t;
    t->status = THREAD_READY;

    return t;
}
