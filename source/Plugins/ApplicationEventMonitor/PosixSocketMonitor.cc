#include "PosixSocketMonitor.h"

#include "dobby.h"
#include "logging/logging.h"

#include <sys/socket.h>

#define LOG_TAG "PosixSocketMonitor"

namespace dobby {

using bind_t = int (*)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static bind_t orig_bind = nullptr;

static int fake_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    INFO_LOG("[-] bind: fd={}", sockfd);
    return orig_bind(sockfd, addr, addrlen);
}

using connect_t = int (*)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
static connect_t orig_connect = nullptr;

static int fake_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    INFO_LOG("[-] connect: fd={}", sockfd);
    return orig_connect(sockfd, addr, addrlen);
}

using send_t = ssize_t (*)(int sockfd, const void *buf, size_t len, int flags);
static send_t orig_send = nullptr;

static ssize_t fake_send(int sockfd, const void *buf, size_t len, int flags) {
    INFO_LOG("[-] send: fd={}, buf: {:p}, len: {}", sockfd, buf, len);
    return orig_send(sockfd, buf, len, flags);
}

using recv_t = ssize_t (*)(int sockfd, void *buf, size_t len, int flags);
static recv_t orig_recv = nullptr;

static ssize_t fake_recv(int sockfd, void *buf, size_t len, int flags) {
    INFO_LOG("[-] recv: fd={}, buf: {:p}, len: {}", sockfd, buf, len);
    return orig_recv(sockfd, buf, len, flags);
}

std::expected<void, PluginError> PosixSocketMonitor::OnEnable() {
    if (hooked_) {
        return {};
    }

    void* bind_addr = DobbySymbolResolver(nullptr, "bind");
    if (bind_addr) {
        DobbyHook(bind_addr, reinterpret_cast<void*>(fake_bind), reinterpret_cast<void**>(&orig_bind));
    }

    void* connect_addr = DobbySymbolResolver(nullptr, "connect");
    if (!connect_addr) {
        return std::unexpected(PluginError::InitializationFailed);
    }
    DobbyHook(connect_addr, reinterpret_cast<void*>(fake_connect), reinterpret_cast<void**>(&orig_connect));

    void* send_addr = DobbySymbolResolver(nullptr, "send");
    if (send_addr) {
        DobbyHook(send_addr, reinterpret_cast<void*>(fake_send), reinterpret_cast<void**>(&orig_send));
    }

    void* recv_addr = DobbySymbolResolver(nullptr, "recv");
    if (recv_addr) {
        DobbyHook(recv_addr, reinterpret_cast<void*>(fake_recv), reinterpret_cast<void**>(&orig_recv));
    }

    hooked_ = true;
    INFO_LOG("PosixSocketMonitor enabled");
    return {};
}

void PosixSocketMonitor::OnDisable() {
    hooked_ = false;
}

} // namespace dobby
