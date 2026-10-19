#include "debug_server.h"

#include <arpa/inet.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <config.h>
#include <screen_manager.h>

namespace DebugServer {

static constexpr uint16_t SERVER_PORT = 6000;
static int serverSocket = -1;
static bool initialized = false;

bool Init() {
    if (initialized) {
        return true;
    }

    if (!NetworkSubsystem::Init()) {
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        NetworkSubsystem::Exit();
        return false;
    }

    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(serverSocket);
        serverSocket = -1;
        NetworkSubsystem::Exit();
        return false;
    }

    initialized = true;
    return true;
}

static void Reply(const sockaddr_in &client, socklen_t clientLen, const char *fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);
    if (len <= 0) {
        return;
    }
    buffer[len] = '\0';

    (void)sendto(serverSocket, buffer, (size_t)len, 0,
                 reinterpret_cast<const sockaddr*>(&client), clientLen);
}

static void Dispatch(const char *cmd, const sockaddr_in &client, socklen_t clientLen) {
    if (strcmp(cmd, "ping") == 0) {
        Reply(client, clientLen, "pong uptime=%u ms", (unsigned)osGetTime());
    }
    else if (strcmp(cmd, "windows") == 0) {
        const UI::ScreenManager &manager = UI::ScreenManager::Instance();
        const UI::Screen *focused = manager.Focused();
        Reply(client, clientLen, "windows count=%d focused=%s",
              manager.WindowCount(), focused? focused->Name() : "none");
    }
    else if (strcmp(cmd, "touch") == 0) {
        const touchPosition &touch = UI::ScreenManager::Instance().LastTouch();
        Reply(client, clientLen, "touch x=%d y=%d", touch.px, touch.py);
    }
    else if (strcmp(cmd, "cfg") == 0) {
        Reply(client, clientLen, "cfg cwd=%s sort=%d filter=%d theme=%s dev=%d",
              cfg.cwd.c_str(), cfg.sort, cfg.filter,
              cfg.dark_theme? "dark" : "light", cfg.dev_options? 1 : 0);
    }
    else {
        Reply(client, clientLen, "unknown: use ping|windows|touch|cfg");
    }
}

void Poll() {
    if (serverSocket < 0) {
        return;
    }

    char buffer[256];
    sockaddr_in client = {};
    socklen_t clientLen = sizeof(client);
    ssize_t received = recvfrom(serverSocket,
                                buffer,
                                sizeof(buffer) - 1,
                                0,
                                reinterpret_cast<sockaddr*>(&client),
                                &clientLen);
    if (received <= 0) {
        return;
    }

    buffer[received] = '\0';

    /* Strip trailing CR/LF from datagrams. */
    while ((received > 0) && ((buffer[received - 1] == '\n') || (buffer[received - 1] == '\r'))) {
        buffer[--received] = '\0';
    }

    if (received <= 0) {
        return;
    }

    Dispatch(buffer, client, clientLen);
}

void Exit() {
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }

    if (initialized) {
        initialized = false;
        NetworkSubsystem::Exit();
    }
}

} // namespace DebugServer
