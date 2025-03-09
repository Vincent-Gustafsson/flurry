#pragma once

#include <stddef.h>

void kmalloc_init();
void *kmalloc(size_t sz);
void kfree(void *ptr);
