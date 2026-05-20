#include "PosixFileMonitor.h"
#include "logging/logging.h"
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <mutex>

#define LOG_TAG "PosixFileMonitor"

namespace dobby {

static std::unordered_map<int, std::string> posix_file_descriptors;
static std::mutex fd_map_mutex;

typedef int (*open_t)(const char *pathname, int flags, ...);
static open_t orig_open = nullptr;

static int fake_open(const char *pathname, int flags, ...) {
    mode_t mode = 0;
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        mode = (mode_t)va_arg(args, int);
        va_end(args);
    }

    int result = orig_open(pathname, flags, mode);
    if (result != -1 && pathname) {
        INFO_LOG("[-] trace open handle: {}", pathname);
        std::lock_guard<std::mutex> lock(fd_map_mutex);
        posix_file_descriptors[result] = pathname;
    }
    return result;
}

typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
static read_t orig_read = nullptr;

static ssize_t fake_read(int fd, void *buf, size_t count) {
    {
        std::lock_guard<std::mutex> lock(fd_map_mutex);
        if (posix_file_descriptors.count(fd)) {
            INFO_LOG("[-] read: {}, buffer: {:p}, size: {}", posix_file_descriptors[fd], buf, count);
        }
    }
    return orig_read(fd, buf, count);
}

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
static write_t orig_write = nullptr;

static ssize_t fake_write(int fd, const void *buf, size_t count) {
    {
        std::lock_guard<std::mutex> lock(fd_map_mutex);
        if (posix_file_descriptors.count(fd)) {
            INFO_LOG("[-] write: {}, buffer: {:p}, size: {}", posix_file_descriptors[fd], buf, count);
        }
    }
    return orig_write(fd, buf, count);
}

typedef int (*close_t)(int fd);
static close_t orig_close = nullptr;

static int fake_close(int fd) {
    {
        std::lock_guard<std::mutex> lock(fd_map_mutex);
        if (posix_file_descriptors.count(fd)) {
            INFO_LOG("[-] close: {}", posix_file_descriptors[fd]);
            posix_file_descriptors.erase(fd);
        }
    }
    return orig_close(fd);
}

std::expected<void, PluginError> PosixFileMonitor::OnEnable() {
    if (hooked_) return {};

    void* open_addr = DobbySymbolResolver(NULL, "open");
    if (!open_addr) return std::unexpected(PluginError::InitializationFailed);
    DobbyHook(open_addr, (void *)fake_open, (void **)&orig_open);

    void* read_addr = DobbySymbolResolver(NULL, "read");
    if (read_addr) DobbyHook(read_addr, (void *)fake_read, (void **)&orig_read);

    void* write_addr = DobbySymbolResolver(NULL, "write");
    if (write_addr) DobbyHook(write_addr, (void *)fake_write, (void **)&orig_write);

    void* close_addr = DobbySymbolResolver(NULL, "close");
    if (close_addr) DobbyHook(close_addr, (void *)fake_close, (void **)&orig_close);

    hooked_ = true;
    INFO_LOG("PosixFileMonitor enabled");
    return {};
}

void PosixFileMonitor::OnDisable() {
    // Unhooking is not yet implemented in Dobby's public API conveniently for this refactor
    hooked_ = false;
}

} // namespace dobby
