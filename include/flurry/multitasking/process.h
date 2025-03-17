#pragma once

#include <stdint.h>
#include <flurry/memory/paging.h>

#include "flurry/multitasking/thread.h"

#define PROCESS_MAX_NAME_LENGTH 64

typedef struct Thread Thread;

// TODO: children and parent processes
typedef struct Process {
    uint64_t pid;
    char name[PROCESS_MAX_NAME_LENGTH];
    PhysAddr pml4;
    Thread* threads;

    struct Process* next;
} Process;

Process* process_create(char* name, PhysAddr pml4);
