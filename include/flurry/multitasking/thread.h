#pragma once

#include <stdint.h>

#include "flurry/multitasking/process.h"
#include "flurry/interrupts/interrupts.h"

#define THREAD_MAX_NAME_LENGTH 64

typedef struct Process Process;

typedef enum {
    THREAD_READY,
    THREAD_BLOCKED,
    THREAD_RUNNING,
    THREAD_DONE,
} ThreadStatus;

typedef struct {
    uint64_t r12, r13, r14, r15, rbp, rbx, rflags;
    uint64_t entry;
} __attribute__((packed)) InitialKStack;

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

    struct Thread* next;
} Thread;

Thread* thread_kcreate(char* name, void (*entry)(void), Process* proc);
