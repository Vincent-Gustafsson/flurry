#include "flurry/syscalls/syscall.h"

#include "log.h"
#include "flurry/cpu/msr.h"

#define MSR_EFER_SYSCALL_ENABLE (1 << 0)
#define MSR_FMASK_IF_DISABLE (1 << 9)



extern void syscall_entry();

void syscall_init() {
    /*
        |Offset|Name|Access byte|
        |0x0|NULL|0|
        |0x8|Kernel Code|0b10011010|
        |0x10|Kernel Data|0b10010010|
        |0x18|User Data|0b11110010|
        |0x20|User Code|0b11111010|

        |MSR|Name|Value|
        |0xC0000081|STAR| 0x0013000800000000 |
        |0xC0000082|LSTAR| address of the handler |
        |0xC0000084|FMASK| 0xFFFFFFFFFFFFFFFD |
     */

    msr_write(MSR_EFER, msr_read(MSR_EFER) | MSR_EFER_SYSCALL_ENABLE);
    msr_write(MSR_STAR, 0x0013000800000000); // TODO: This value is basically magic. Don't fully understand it yet.
    //TODO: msr_write(MSR_LSTAR, syscall_entry);
    msr_write(MSR_FMASK, msr_read(MSR_FMASK) | MSR_FMASK_IF_DISABLE);
    logln(LOG_INFO, "[SYSCALL] Initialized");
}
