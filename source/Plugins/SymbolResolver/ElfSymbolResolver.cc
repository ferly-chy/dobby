#include "ElfSymbolResolver.h"

#include <elfio/elfio.hpp>
#include "logging/logging.h"

#define LOG_TAG "ElfSymbolResolver"

namespace dobby {

struct ElfSymbolResolver::Context {
    ELFIO::elfio reader;
    uintptr_t link_vaddr = 0;
};

ElfSymbolResolver::ElfSymbolResolver(const std::string &library_path) : path_(library_path), ctx_(std::make_unique<Context>()) {
}

ElfSymbolResolver::~ElfSymbolResolver() = default;

bool ElfSymbolResolver::Initialize() {
    if (initialized_) return true;

    if (!ctx_->reader.load(path_)) {
        ERROR_LOG("Failed to load ELF: {}", path_);
        return false;
    }

    // Find link_vaddr from the first PT_LOAD segment
    for (int i = 0; i < ctx_->reader.segments.size(); ++i) {
        if (ctx_->reader.segments[i]->get_type() == ELFIO::PT_LOAD) {
            ctx_->link_vaddr = ctx_->reader.segments[i]->get_virtual_address();
            break;
        }
    }

    initialized_ = true;
    return true;
}

void* ElfSymbolResolver::Resolve(const std::string &symbol_name, uintptr_t actual_load_address) {
    if (!Initialize()) return nullptr;

    uintptr_t st_value = 0;
    bool found = false;

    auto resolve_from_section = [&](const auto& section) {
        ELFIO::symbol_section_accessor symbols(ctx_->reader, section);
        for (int j = 0; j < symbols.get_symbols_num(); ++j) {
            std::string name;
            ELFIO::Elf64_Addr value;
            ELFIO::Elf_Xword size;
            unsigned char bind;
            unsigned char type;
            ELFIO::Elf_Half section_index;
            unsigned char other;
            if (symbols.get_symbol(j, name, value, size, bind, type, section_index, other)) {
                if (name == symbol_name) {
                    st_value = static_cast<uintptr_t>(value);
                    return true;
                }
            }
        }
        return false;
    };

    // Search in all symbol sections
    for (int i = 0; i < ctx_->reader.sections.size(); ++i) {
        auto section = ctx_->reader.sections[i];
        if (section->get_type() == ELFIO::SHT_SYMTAB || section->get_type() == ELFIO::SHT_DYNSYM) {
            if (resolve_from_section(section)) {
                found = true;
                break;
            }
        }
    }

    if (found) {
        if (actual_load_address) {
            // Formula: absolute_address = actual_load_address + (st_value - link_vaddr)
            return reinterpret_cast<void *>(actual_load_address + (st_value - ctx_->link_vaddr));
        }
        return reinterpret_cast<void *>(st_value);
    }

    return nullptr;
}

} // namespace dobby
