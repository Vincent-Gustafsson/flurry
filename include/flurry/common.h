#pragma once
#include "flurry/log/tty.h"

void kpanic();

#define kassert(expr, msg) \
    do { \
        if (!(expr)) { \
            kprintf("KASSERT: %s\n", msg); \
            kpanic(); \
        } \
    } while (0)

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
