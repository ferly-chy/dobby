#include "ElfSymbolResolver.h"

#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <memory>

#include "logging/logging.h"

#define LOG_TAG "ElfSymbolResolver"

namespace dobby {

struct ElfSymbolResolver::Context {
    ElfW(Sym) * symtab = nullptr;
    const char *strtab = nullptr;
    size_t sym_count = 0;

    ElfW(Sym) * dynsymtab = nullptr;
    const char *dynstrtab = nullptr;
    size_t dynsym_count = 0;
};

ElfSymbolResolver::ElfSymbolResolver(const std::string &library_path) : path_(library_path), ctx_(std::make_unique<Context>()) {
}

ElfSymbolResolver::~ElfSymbolResolver() {
    if (mmap_addr_) {
        munmap(mmap_addr_, mmap_size_);
    }
}

bool ElfSymbolResolver::Initialize() {
    if (initialized_) return true;

    int fd = open(path_.c_str(), O_RDONLY);
    if (fd < 0) {
        ERROR_LOG("Failed to open library: {}", path_);
        return false;
    }

    struct stat s;
    if (fstat(fd, &s) != 0) {
        close(fd);
        return false;
    }
    mmap_size_ = s.st_size;

    mmap_addr_ = mmap(nullptr, mmap_size_, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (mmap_addr_ == MAP_FAILED) {
        mmap_addr_ = nullptr;
        ERROR_LOG("Failed to mmap library: {}", path_);
        return false;
    }

    auto *ehdr = reinterpret_cast<ElfW(Ehdr) *>(mmap_addr_);
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        ERROR_LOG("Invalid ELF header: {}", path_);
        return false;
    }

    uintptr_t ehdr_addr = reinterpret_cast<uintptr_t>(ehdr);

    // Handle load bias from program headers
    auto *phdr = reinterpret_cast<ElfW(Phdr) *>(ehdr_addr + ehdr->e_phoff);
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            load_bias_ = ehdr_addr - (phdr[i].p_vaddr - phdr[i].p_offset);
            break;
        } else if (phdr[i].p_type == PT_PHDR) {
            load_bias_ = reinterpret_cast<uintptr_t>(phdr) - phdr[i].p_vaddr;
            break;
        }
    }

    // Handle sections to find symbol tables
    auto *shdr = reinterpret_cast<ElfW(Shdr) *>(ehdr_addr + ehdr->e_shoff);
    auto *shstr_sh = &shdr[ehdr->e_shstrndx];
    const char *shstrtab = reinterpret_cast<const char *>(ehdr_addr + shstr_sh->sh_offset);

    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB) {
            ctx_->symtab = reinterpret_cast<ElfW(Sym) *>(ehdr_addr + shdr[i].sh_offset);
            ctx_->sym_count = shdr[i].sh_size / sizeof(ElfW(Sym));
        } else if (shdr[i].sh_type == SHT_STRTAB && strcmp(shstrtab + shdr[i].sh_name, ".strtab") == 0) {
            ctx_->strtab = reinterpret_cast<const char *>(ehdr_addr + shdr[i].sh_offset);
        } else if (shdr[i].sh_type == SHT_DYNSYM) {
            ctx_->dynsymtab = reinterpret_cast<ElfW(Sym) *>(ehdr_addr + shdr[i].sh_offset);
            ctx_->dynsym_count = shdr[i].sh_size / sizeof(ElfW(Sym));
        } else if (shdr[i].sh_type == SHT_STRTAB && strcmp(shstrtab + shdr[i].sh_name, ".dynstr") == 0) {
            ctx_->dynstrtab = reinterpret_cast<const char *>(ehdr_addr + shdr[i].sh_offset);
        }
    }

    initialized_ = true;
    return true;
}

void* ElfSymbolResolver::Resolve(const std::string &symbol_name, uintptr_t actual_load_address) {
    if (!Initialize()) return nullptr;

    void* symbol_offset = nullptr;

    // Search in .symtab
    if (ctx_->symtab && ctx_->strtab) {
        for (size_t i = 0; i < ctx_->sym_count; ++i) {
            ElfW(Sym) *sym = &ctx_->symtab[i];
            if (sym->st_name != 0 && strcmp(ctx_->strtab + sym->st_name, symbol_name.c_str()) == 0) {
                symbol_offset = reinterpret_cast<void *>(sym->st_value);
                break;
            }
        }
    }

    // Search in .dynsym
    if (!symbol_offset && ctx_->dynsymtab && ctx_->dynstrtab) {
        for (size_t i = 0; i < ctx_->dynsym_count; ++i) {
            ElfW(Sym) *sym = &ctx_->dynsymtab[i];
            if (sym->st_name != 0 && strcmp(ctx_->dynstrtab + sym->st_name, symbol_name.c_str()) == 0) {
                symbol_offset = reinterpret_cast<void *>(sym->st_value);
                break;
            }
        }
    }

    if (symbol_offset && actual_load_address) {
        // Calculate absolute address: load_address + (symbol_value - link_vaddr)
        // link_vaddr is handled via load_bias in our calculation
        uintptr_t file_mem = reinterpret_cast<uintptr_t>(mmap_addr_);
        return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(symbol_offset) + actual_load_address - (file_mem - load_bias_));
    }

    return symbol_offset;
}

} // namespace dobby
