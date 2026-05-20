#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

#include "PlatformUtil/ProcessRuntimeUtility.h"

namespace dobby {

class ElfSymbolResolver {
public:
    struct SymbolInfo {
        std::string name;
        uintptr_t address;
        size_t size;
    };

    explicit ElfSymbolResolver(const std::string& library_path);
    ~ElfSymbolResolver();

    // Disable copy
    ElfSymbolResolver(const ElfSymbolResolver&) = delete;
    ElfSymbolResolver& operator=(const ElfSymbolResolver&) = delete;

    // Resolve symbol by name. actual_load_address is the base address where the library is loaded in memory.
    void* Resolve(const std::string& symbol_name, uintptr_t actual_load_address = 0);

private:
    bool Initialize();
    void* ResolveInternal(const std::string& symbol_name);

    std::string path_;
    void* mmap_addr_ = nullptr;
    size_t mmap_size_ = 0;
    uintptr_t load_bias_ = 0;
    bool initialized_ = false;

    // Internal ELF context data
    struct Context;
    std::unique_ptr<Context> ctx_;
};

} // namespace dobby
