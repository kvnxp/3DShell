#include "network.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <malloc.h>

#include <netinet/in.h>

#include <sys/socket.h>
#include <unistd.h>

namespace NetworkSubsystem {

static constexpr size_t SOC_BUFFER_SIZE = 0x100000;
static void* socBuffer = nullptr;
static bool initialized = false;
static int refCount = 0;

bool Init() {
    if (refCount > 0) {
        ++refCount;
        return true;
    }

    socBuffer = memalign(0x1000, SOC_BUFFER_SIZE);
    if (!socBuffer) {
        return false;
    }

    Result rc = socInit(reinterpret_cast<u32*>(socBuffer), static_cast<u32>(SOC_BUFFER_SIZE));
    if (rc != 0) {
        free(socBuffer);
        socBuffer = nullptr;
        return false;
    }

    initialized = true;
    refCount = 1;
    return true;
}

void Exit() {
    if (refCount <= 0) {
        return;
    }
    --refCount;
    if (refCount > 0) {
        return;
    }

    if (initialized) {
        socExit();
        initialized = false;
    }

    if (socBuffer) {
        free(socBuffer);
        socBuffer = nullptr;
    }
}

bool IsInitialized() {
    return initialized;
}

} // namespace NetworkSubsystem
