#pragma once

#include <stdint.h>

#include "flurry/multitasking/process.h"
#include "flurry/interrupts/interrupts.h"

#define THREAD_MAX_NAME_LENGTH 64

typedef struct Process Process;

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_DONE
} ThreadStatus;


typedef struct Thread {
    uint64_t tid;
    Process* proc;
    char name[THREAD_MAX_NAME_LENGTH];

    ThreadStatus status;
    //InterruptCtx ctx;
    uint64_t rsp;

    uintptr_t k_rsp;
    uintptr_t k_base;

    struct Thread* next;
} Thread;

Thread* thread_kcreate(char* name, Process* proc, void (*entry)(void));
