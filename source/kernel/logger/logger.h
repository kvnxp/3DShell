#ifndef KERNEL_NETWORK_LOGGER_H
#define KERNEL_NETWORK_LOGGER_H

#include <3ds.h>

namespace NetworkLogger {
    bool Init();
    void Log(const char* tag, const char* message);
    void Printf(const char* fmt, ...);
    void PrintfTagged(const char* tag, const char* fmt, ...);
    void Exit();
}

#endif // KERNEL_NETWORK_LOGGER_H
