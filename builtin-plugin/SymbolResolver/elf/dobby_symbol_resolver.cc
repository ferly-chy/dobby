#include "SymbolResolver/dobby_symbol_resolver.h"
#include "Plugins/SymbolResolver/ElfSymbolResolver.h"
#include "PlatformUtil/ProcessRuntimeUtility.h"
#include <dlfcn.h>
#include <vector>

using namespace dobby;

void *resolve_elf_internal_symbol(const char *library_name, const char *symbol_name) {
    if (library_name) {
        RuntimeModule module = ProcessRuntimeUtility::GetProcessModule(library_name);
        if (module.load_address) {
            ElfSymbolResolver resolver(module.path);
            return resolver.Resolve(symbol_name, reinterpret_cast<uintptr_t>(module.load_address));
        }
    }

    auto module_map = ProcessRuntimeUtility::GetProcessModuleMap();
    for (const auto& module : module_map) {
        if (module.load_address) {
            ElfSymbolResolver resolver(module.path);
            void* result = resolver.Resolve(symbol_name, reinterpret_cast<uintptr_t>(module.load_address));
            if (result) return result;
        }
    }
    return nullptr;
}

extern "C" void *DobbySymbolResolver(const char *image_name, const char *symbol_name) {
    void *result = dlsym(RTLD_DEFAULT, symbol_name);
    if (result) return result;

    return resolve_elf_internal_symbol(image_name, symbol_name);
}
