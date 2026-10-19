#ifndef KERNEL_NETWORK_H
#define KERNEL_NETWORK_H

#include <3ds.h>

/* Reference-counted SOC subsystem: socInit() may only run once per app,
 * so the kernel logger, the debug server and the shell's curl/netstack
 * share one initialization. */
namespace NetworkSubsystem {
    bool Init();
    void Exit();
    bool IsInitialized();
}

#endif // KERNEL_NETWORK_H
