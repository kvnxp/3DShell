#ifndef KERNEL_DEBUG_SERVER_H
#define KERNEL_DEBUG_SERVER_H

#include <3ds.h>

#include "network/network.h"

/* adb-shell-like debug channel: the PC sends ASCII commands to UDP
 * <3ds-ip>:6000; the console answers at the command's source address.
 * Polled once per frame from the main loop (single read). */
namespace DebugServer {
    bool Init();
    void Poll();
    void Exit();
}

#endif // KERNEL_DEBUG_SERVER_H
