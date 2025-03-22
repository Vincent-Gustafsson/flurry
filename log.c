#include "include/log.h"
#include "include/flurry/asm_wrappers.h"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0

// Compile nanoprintf in this translation unit.
#define NANOPRINTF_IMPLEMENTATION
#include "include/nanoprintf.h"



static void qemu_debug_putc(char c) {
    write_portb(0xE9, (unsigned char)c);
}

// Output a null-terminated string via the debug port
static void qemu_debug_print(const char* str) {
    while (*str)
        qemu_debug_putc(*str++);
}

// ANSI escape codes for colors
#define COLOR_RESET "\033[0m"
#define COLOR_DEBUG "\033[90m"  // Grey
#define COLOR_INFO  "\033[36m"  // Cyan
#define COLOR_ERROR "\033[31m"  // Red

// Internal helper: prints the log with color, level prefix, and optional newline.
static void qemu_debug_log_internal(LogLevel level, int append_newline, const char* fmt, va_list args) {
    char buffer[256];  // Adjust as needed
    const char *color, *level_str;

    switch (level) {
        case LOG_DEBUG:
            color = COLOR_DEBUG;
        level_str = "[DEBUG] ";
        break;
        case LOG_INFO:
            color = COLOR_INFO;
        level_str = "[INFO] ";
        break;
        case LOG_ERROR:
            color = COLOR_ERROR;
        level_str = "[ERROR] ";
        break;
        default:
            color = "";
        level_str = "";
        break;
    }

    // Output color code and level prefix.
    qemu_debug_print(color);
    qemu_debug_print(level_str);

    // Format the message into buffer.
    npf_vsnprintf(buffer, sizeof(buffer), fmt, args);
    qemu_debug_print(buffer);

    // Reset color.
    qemu_debug_print(COLOR_RESET);

    // Append newline if requested.
    if (append_newline) {
        qemu_debug_print("\n");
    }
}

// Public function: log without automatically adding a newline.
void log(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    qemu_debug_log_internal(level, 0, fmt, args);
    va_end(args);
}

// Public function: log and append a newline at the end.
void logln(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    qemu_debug_log_internal(level, 1, fmt, args);
    va_end(args);
}
