#include "ImportTableResolver.h"

#include <elfio/elfio.hpp>
#include "logging/logging.h"

#define LOG_TAG "ImportTableResolver"

namespace dobby {

struct ImportTableResolver::Context {
    ELFIO::elfio reader;
    uintptr_t link_vaddr = 0;
};

ImportTableResolver::ImportTableResolver(const std::string &library_path) : path_(library_path), ctx_(std::make_unique<Context>()) {
}

ImportTableResolver::~ImportTableResolver() = default;

bool ImportTableResolver::Initialize() {
    if (initialized_) return true;

    if (!ctx_->reader.load(path_)) {
        ERROR_LOG("Failed to load ELF: {}", path_);
        return false;
    }

    for (int i = 0; i < ctx_->reader.segments.size(); ++i) {
        if (ctx_->reader.segments[i]->get_type() == ELFIO::PT_LOAD) {
            ctx_->link_vaddr = ctx_->reader.segments[i]->get_virtual_address();
            break;
        }
    }

    initialized_ = true;
    return true;
}

DobbyStatus ImportTableResolver::Replace(const std::string &symbol_name, void *fake_func, void **orig_func, uintptr_t actual_load_address) {
    if (!Initialize()) return kDobbyFailed;

    ELFIO::section *dynsym_sec = nullptr;
    for (int i = 0; i < ctx_->reader.sections.size(); ++i) {
        if (ctx_->reader.sections[i]->get_type() == ELFIO::SHT_DYNSYM) {
            dynsym_sec = ctx_->reader.sections[i];
            break;
        }
    }

    if (!dynsym_sec) {
        ERROR_LOG("Failed to find .dynsym section");
        return kDobbyFailed;
    }

    ELFIO::symbol_section_accessor symbols(ctx_->reader, dynsym_sec);
    int symbol_index = -1;
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
                symbol_index = j;
                break;
            }
        }
    }

    if (symbol_index == -1) {
        ERROR_LOG("Failed to find symbol {} in .dynsym", symbol_name);
        return kDobbyFailed;
    }

    for (int i = 0; i < ctx_->reader.sections.size(); ++i) {
        auto section = ctx_->reader.sections[i];
        if (section->get_type() == ELFIO::SHT_REL || section->get_type() == ELFIO::SHT_RELA) {
            ELFIO::relocation_section_accessor rels(ctx_->reader, section);
            for (int j = 0; j < rels.get_entries_num(); ++j) {
                ELFIO::Elf64_Addr offset;
                ELFIO::Elf_Word symbol;
                ELFIO::Elf_Word type;
                ELFIO::Elf_Sxword addend;
                if (rels.get_entry(j, offset, symbol, type, addend)) {
                    if (symbol == (ELFIO::Elf_Word)symbol_index) {
                        // Found the relocation entry
                        uintptr_t got_entry_addr = actual_load_address + (static_cast<uintptr_t>(offset) - ctx_->link_vaddr);
                        
                        if (orig_func) {
                            *orig_func = *reinterpret_cast<void **>(got_entry_addr);
                        }

                        DobbyCodePatch(reinterpret_cast<void *>(got_entry_addr), reinterpret_cast<uint8_t *>(&fake_func), sizeof(void *));
                        DEBUG_LOG("Patched GOT entry for {} at {:p}", symbol_name, (void *)got_entry_addr);
                        return kDobbySuccess;
                    }
                }
            }
        }
    }

    ERROR_LOG("Failed to find relocation for symbol {}", symbol_name);
    return kDobbyFailed;
}

} // namespace dobby
