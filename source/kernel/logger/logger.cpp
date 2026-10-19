#include "logger.h"
#include "network/network.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace NetworkLogger {

static constexpr uint16_t HOST_PORT = 5000; // UDP broadcast port

static int udpSocket = -1;
static sockaddr_in destAddr = {};
static bool initialized = false;

bool Init() {
    if (initialized) {
        return true;
    }

    if (!NetworkSubsystem::Init()) {
        return false;
    }

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        NetworkSubsystem::Exit();
        return false;
    }

    int flags = fcntl(udpSocket, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(udpSocket, F_SETFL, flags | O_NONBLOCK);
    }

    // Broadcast: any host on the LAN receives with `nc -u -l 5000`
    // (3dslink -s does not carry logs; this is our own UDP channel).
    int one = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(HOST_PORT);
    inet_pton(AF_INET, "255.255.255.255", &destAddr.sin_addr);

    initialized = true;
    return true;
}

void Log(const char* tag, const char* message) {
    if (!initialized || udpSocket < 0 || !tag || !message) {
        return;
    }

    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer), "[%s] %s\n", tag, message);
    if (len <= 0) {
        return;
    }
    if (static_cast<size_t>(len) >= sizeof(buffer)) {
        len = sizeof(buffer) - 1;
        buffer[len] = '\0';
    }

    sendto(udpSocket,
           buffer,
           static_cast<size_t>(len),
           0,
           reinterpret_cast<sockaddr*>(&destAddr),
           sizeof(destAddr));
}

void PrintfTagged(const char* tag, const char* fmt, ...) {
    if (!fmt) {
        return;
    }

    if (!initialized || udpSocket < 0 || !tag) {
        return;
    }

    char buffer[512];
    va_list args2;
    va_start(args2, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args2);
    va_end(args2);
    if (len <= 0) {
        return;
    }
    if (static_cast<size_t>(len) >= sizeof(buffer)) {
        len = sizeof(buffer) - 1;
        buffer[len] = '\0';
    }
    Log(tag, buffer);
}

void Printf(const char* fmt, ...) {
    if (!fmt) {
        return;
    }

    if (!initialized || udpSocket < 0) {
        return;
    }

    char buffer[512];
    va_list args2;
    va_start(args2, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args2);
    va_end(args2);
    if (len <= 0) {
        return;
    }
    if (static_cast<size_t>(len) >= sizeof(buffer)) {
        len = sizeof(buffer) - 1;
        buffer[len] = '\0';
    }
    Log("KERNEL", buffer);
}

void Exit() {
    if (udpSocket >= 0) {
        close(udpSocket);
        udpSocket = -1;
    }

    if (initialized) {
        NetworkSubsystem::Exit();
        initialized = false;
    }
}

} // namespace NetworkLogger
