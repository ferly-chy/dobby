#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <memory>

namespace dobby {

class ElfSymbolResolver {
public:
    explicit ElfSymbolResolver(const std::string& library_path);
    ~ElfSymbolResolver();

    // Disable copy
    ElfSymbolResolver(const ElfSymbolResolver&) = delete;
    ElfSymbolResolver& operator=(const ElfSymbolResolver&) = delete;

    // Resolve symbol by name. actual_load_address is the base address where the library is loaded in memory.
    void* Resolve(const std::string& symbol_name, uintptr_t actual_load_address = 0);

private:
    bool Initialize();

    std::string path_;
    bool initialized_ = false;

    // Internal ELF context data
    struct Context;
    std::unique_ptr<Context> ctx_;
};

} // namespace dobby
