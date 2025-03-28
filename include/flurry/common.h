#pragma once

#include <stdint.h>

#include "log.h"

static inline uint64_t from_ms(uint64_t ms) { return ms * 1000000ULL; }

typedef uint64_t PhysAddr;

void kpanic();

/* clamp between [min, max] */
uint64_t clamp(uint64_t value, uint64_t min, uint64_t max);

#define kassert(expr, msg) \
    do { \
        if (!(expr)) { \
            logln(LOG_ERROR, "KASSERT: %s", msg); \
            kpanic(); \
        } \
    } while (0)

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
