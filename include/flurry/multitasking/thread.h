#pragma once

#include <stdint.h>

#include "events.h"
#include "flurry/multitasking/process.h"
#include "flurry/interrupts/interrupts.h"

#define THREAD_MAX_NAME_LENGTH 64

typedef struct Process Process;

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_BLOCKED,
    THREAD_DONE,
} ThreadStatus;

typedef struct Thread Thread;

typedef struct {
    uint64_t r12, r13, r14, r15, rbp, rbx, rflags;

    void (*init_thread)(Thread *prev);
    void (*entry)();
} __attribute__((packed)) InitialKStack;

typedef struct Event Event;

typedef struct Thread {
    uintptr_t rsp;
    //uintptr_t rsp0;
    PhysAddr cr3;

    uintptr_t rsp_base;
    //uintptr_t rsp0_base;

    uintptr_t tid;
    Process* process;
    char name[THREAD_MAX_NAME_LENGTH];
    ThreadStatus status;
    Event* event;

    uint64_t sleep_until;

    struct Thread* next;
} Thread;

Thread* thread_kcreate(char* name, void (*entry)(void), Process* proc);
