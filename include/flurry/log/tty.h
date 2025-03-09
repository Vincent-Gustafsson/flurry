#pragma once

#include <stdint.h>

void tty_init(uint32_t width, uint32_t height, uint32_t *fb);
void tty_putchar_at(uint32_t x, uint32_t y, char c);
void tty_putchar(char c);
void kputchar(char c);
void kputs(const char *s);
void kprintf(char* format, ...);
