#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "flurry/log/tty.h"
#include "font.h"

// Stolen from: https://github.com/lolzdev/hydra/blob/x86_64/sys/log/fb.h

static uint32_t char_width = 0;
static uint32_t char_height = 0;
static uint32_t fb_width;
static uint32_t fb_height;
static size_t row;
static size_t column;
static uint32_t *framebuffer;

void tty_init(uint32_t width, uint32_t height, uint32_t *fb)
{
	fb_width = width;
	fb_height = height;
	framebuffer = fb;
	char_width = width / 9;
	char_height = height / 17;
	row = 0;
	column = 0;
}

void tty_kputchar_at(uint32_t x, uint32_t y, char c )
{
	if (x >= fb_width) x = 0;
	if (y >= fb_height) y = 0;
	x *= 9;
	y *= 17;
	uint32_t original_x = x;
	const char *data = (char*) font_8x16[c - 32];
	for (int j=0; j<16; j++) {
		char byte = data[j];
		for (int i=8; i>-1; i--) {
			uint32_t index = y*fb_width + x;
			framebuffer[index] = ((byte >> i) & 0x1) ? 0xffffff : 0x0;
			x++;
		}
		x = original_x;
		y++;
	}
}


void tty_kputchar(char c)
{
	if (row >= fb_height) row = 0;
	if (column >= fb_width) column = 0;
	if (c == '\n') {
		if (row > char_height) {
			row = 0;
		}
		column = 0;
		row++;
		return;
	}

	tty_kputchar_at(column, row, c);
	column++;
	if (column > char_width) {
		column = 0;
		row++;
	}
}

void kputchar(char c)
{
	tty_kputchar(c);
}

void kputs(const char *s)
{
	while (*s != '\0') {
		kputchar(*s);
		s++;
	}
}

static void reverse(char *str, int length) {
	int start = 0, end = length - 1;
	char temp;
	while (start < end) {
		temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
}

static char* itoa(uint64_t value, char *str, int base) {
	if (base < 2 || base > 36) {
		*str = '\0'; // Return empty string for unsupported bases
		return str;
	}

	int i = 0;
	do {
		int remainder = value % base;
		str[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';
		value = value / base;
	} while (value != 0);

	reverse(str, i);
	str[i] = '\0'; // Null-terminate the string

	return str;
}


void kprintf(char* format, ...) {
	va_list args;
	va_start(args, format);

	char buffer[21];

	while(*format != '\0') {
		if(*format != '%') {
			kputchar(*format);
			format++;
			continue;
		}

		format++;

		if(*format == '\0')
			break;

		switch(*format) {
			case 's': kputs(va_arg(args, char *)); break;
			case 'c': kputchar(va_arg(args, int)); break;
			case 'd': kputs(itoa(va_arg(args, uint64_t), buffer, 10)); break;
			case 'x': kputs(itoa(va_arg(args, uint64_t), buffer, 16)); break;
		}
		format++;
	}
}
