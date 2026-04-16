#include "SymbolResolver/dobby_symbol_resolver.h"
#include "dobby/common.h"
#include "common/mmap_file_util.h"
#include "PlatformUtil/ProcessRuntime.h"

#include <elf.h>
#include <dlfcn.h>
#include <link.h>
#include <string.h>
#include <vector>

#undef LOG_TAG
#define LOG_TAG "DobbySymbolResolver"

struct dl_iterate_context {
  const char *image_name;
  const char *symbol_name;
  void *result;
};

static void *resolve_from_elf_file(const char *path, uintptr_t load_bias, const char *symbol_name) {
  MmapFileManager mmap_manager(path);
  void *file_mem = mmap_manager.map();
  if (!file_mem) return nullptr;

  void *result = nullptr;
  auto *ehdr = (ElfW(Ehdr) *)file_mem;
  auto *shdr = (ElfW(Shdr) *)((uintptr_t)file_mem + ehdr->e_shoff);
  char *shstrtab = (char *)((uintptr_t)file_mem + shdr[ehdr->e_shstrndx].sh_offset);

  for (int i = 0; i < ehdr->e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM) {
      auto *symtab = (ElfW(Sym) *)((uintptr_t)file_mem + shdr[i].sh_offset);
      size_t sym_count = shdr[i].sh_size / sizeof(ElfW(Sym));
      
      // Find the associated string table
      if (shdr[i].sh_link >= ehdr->e_shnum) continue;
      char *strtab = (char *)((uintptr_t)file_mem + shdr[shdr[i].sh_link].sh_offset);

      for (size_t j = 0; j < sym_count; j++) {
        const char *name = strtab + symtab[j].st_name;
        if (strcmp(name, symbol_name) == 0) {
          if (symtab[j].st_shndx != SHN_UNDEF) {
            result = (void *)(load_bias + symtab[j].st_value);
            goto cleanup;
          }
        }
      }
    }
  }

cleanup:
  return result;
}

static int dl_iterate_callback(struct dl_phdr_info *info, size_t size, void *data) {
  auto *ctx = static_cast<dl_iterate_context *>(data);

  if (ctx->image_name != nullptr) {
    if (info->dlpi_name == nullptr || strstr(info->dlpi_name, ctx->image_name) == nullptr) {
      return 0;
    }
  }

  // 1. Try resolving from file directly (covers both .symtab and .dynsym)
  if (info->dlpi_name && info->dlpi_name[0] != '\0') {
    void *res = resolve_from_elf_file(info->dlpi_name, info->dlpi_addr, ctx->symbol_name);
    if (res) {
      ctx->result = res;
      return 1;
    }
  }

  return 0;
}

PUBLIC void *DobbySymbolResolver(const char *image_name, const char *symbol_name) {
  if (symbol_name == nullptr) return nullptr;

  // 1. Try dlsym first (fastest for public symbols)
  void *result = dlsym(RTLD_DEFAULT, symbol_name);
  if (result) return result;

  // 2. Use dl_iterate_phdr to find modules and parse their ELF files for internal symbols
  dl_iterate_context ctx = {image_name, symbol_name, nullptr};
  dl_iterate_phdr(dl_iterate_callback, &ctx);

  return ctx.result;
}
