#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "dobby.h"

namespace dobby {

class ImportTableResolver {
public:
    explicit ImportTableResolver(const std::string& library_path);
    ~ImportTableResolver();

    // Disable copy
    ImportTableResolver(const ImportTableResolver&) = delete;
    ImportTableResolver& operator=(const ImportTableResolver&) = delete;

    // Replace symbol in GOT. actual_load_address is the base address of the library.
    DobbyStatus Replace(const std::string& symbol_name, void* fake_func, void** orig_func, uintptr_t actual_load_address);

private:
    bool Initialize();

    std::string path_;
    bool initialized_ = false;

    struct Context;
    std::unique_ptr<Context> ctx_;
};

} // namespace dobby
