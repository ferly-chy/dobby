#include "DynamicLoaderMonitor.h"

#include "dobby.h"
#include "logging/logging.h"

#include <dlfcn.h>
#include <mutex>
#include <string>
#include <unordered_map>

#define LOG_TAG "DynamicLoaderMonitor"

namespace dobby {

static std::unordered_map<void*, std::string> traced_dlopen_handles;
static std::mutex dlopen_map_mutex;

using dlopen_t = void* (*)(const char* file, int mode);
static dlopen_t orig_dlopen = nullptr;

static void* fake_dlopen(const char* file, int mode) {
    void* result = orig_dlopen(file, mode);
    if (result != nullptr && file != nullptr) {
        INFO_LOG("[-] dlopen handle: {}", file);
        std::lock_guard<std::mutex> lock(dlopen_map_mutex);
        traced_dlopen_handles[result] = file;
    }
    return result;
}

using loader_dlopen_t = void* (*)(const char* filename, int flags, const void* caller_addr);
static loader_dlopen_t orig_loader_dlopen = nullptr;

static void* fake_loader_dlopen(const char* filename, int flags, const void* caller_addr) {
    void* result = orig_loader_dlopen(filename, flags, caller_addr);
    if (result != nullptr && filename != nullptr) {
        INFO_LOG("[-] dlopen handle: {}", filename);
        std::lock_guard<std::mutex> lock(dlopen_map_mutex);
        traced_dlopen_handles[result] = filename;
    }
    return result;
}

static std::string GetTracedFilename(void* handle, bool remove) {
    std::lock_guard<std::mutex> lock(dlopen_map_mutex);
    auto it = traced_dlopen_handles.find(handle);
    if (it == traced_dlopen_handles.end()) {
        return {};
    }

    std::string filename = it->second;
    if (remove) {
        traced_dlopen_handles.erase(it);
    }
    return filename;
}

using dlsym_t = void* (*)(void* handle, const char* symbol);
static dlsym_t orig_dlsym = nullptr;

static void* fake_dlsym(void* handle, const char* symbol) {
    std::string traced_filename = GetTracedFilename(handle, false);
    if (!traced_filename.empty() && symbol != nullptr) {
        INFO_LOG("[-] dlsym: {}, symbol: {}", traced_filename, symbol);
    }
    return orig_dlsym(handle, symbol);
}

using dlclose_t = int (*)(void* handle);
static dlclose_t orig_dlclose = nullptr;

static int fake_dlclose(void* handle) {
    std::string traced_filename = GetTracedFilename(handle, true);
    if (!traced_filename.empty()) {
        INFO_LOG("[-] dlclose: {}", traced_filename);
    }
    return orig_dlclose(handle);
}

std::expected<void, PluginError> DynamicLoaderMonitor::OnEnable() {
    if (hooked_) {
        return {};
    }

#if defined(__ANDROID__)
    void* loader_dlopen_addr = DobbySymbolResolver(nullptr, "__loader_dlopen");
    if (loader_dlopen_addr) {
        DobbyHook(loader_dlopen_addr, reinterpret_cast<void*>(fake_loader_dlopen), reinterpret_cast<void**>(&orig_loader_dlopen));
    }
#endif

    void* dlopen_addr = DobbySymbolResolver(nullptr, "dlopen");
    if (!dlopen_addr) {
        return std::unexpected(PluginError::InitializationFailed);
    }
    DobbyHook(dlopen_addr, reinterpret_cast<void*>(fake_dlopen), reinterpret_cast<void**>(&orig_dlopen));

    DobbyHook(reinterpret_cast<void*>(dlsym), reinterpret_cast<void*>(fake_dlsym), reinterpret_cast<void**>(&orig_dlsym));
    DobbyHook(reinterpret_cast<void*>(dlclose), reinterpret_cast<void*>(fake_dlclose), reinterpret_cast<void**>(&orig_dlclose));

    hooked_ = true;
    INFO_LOG("DynamicLoaderMonitor enabled");
    return {};
}

void DynamicLoaderMonitor::OnDisable() {
    {
        std::lock_guard<std::mutex> lock(dlopen_map_mutex);
        traced_dlopen_handles.clear();
    }
    hooked_ = false;
}

} // namespace dobby
