#pragma once

#include <stdint.h>

#include "log/tty.h"

typedef uint64_t PhysAddr;

void kpanic();

#define kassert(expr, msg) \
    do { \
        if (!(expr)) { \
            kprintf("KASSERT: %s\n", msg); \
            kpanic(); \
        } \
    } while (0)

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
