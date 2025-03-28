#pragma once

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_NORMAL,
    LOG_ERROR
} LogLevel;

void log(LogLevel level, const char* fmt, ...);
void logln(LogLevel level, const char* fmt, ...);
