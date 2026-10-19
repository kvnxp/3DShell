#include "kernel.h"

#include "debug/debug_server.h"
#include "logger/logger.h"

namespace Kernel {

static bool initialized = false;

bool Init() {
    if (initialized) {
        return true;
    }

    NetworkLogger::Init();
    initialized = DebugServer::Init();
    return initialized;
}

void Poll() {
    if (initialized) {
        DebugServer::Poll();
    }
}

void Exit() {
    if (!initialized) {
        return;
    }

    initialized = false;
    DebugServer::Exit();
}

} // namespace Kernel
