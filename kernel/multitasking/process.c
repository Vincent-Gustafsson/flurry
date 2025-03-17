#include "flurry/multitasking/process.h"


#include "flurry/string.h"
#include "flurry/memory/kmalloc.h"


_Atomic uint64_t pid_counter = 0;

Process* process_create(char* name, PhysAddr pml4) {
    Process* process = kmalloc(sizeof(Process));

    process->pid = pid_counter++;
    process->pml4 = pml4;
    strcpy(process->name, name);

    return process;
}
