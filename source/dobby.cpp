#include "dobby/dobby_internal.h"
#include "Interceptor.h"
#include "Plugins/Plugin.h"
#include "Plugins/SymbolResolver/ElfSymbolResolver.h"
#include "Plugins/SymbolResolver/ImportTableResolver.h"
#include "PlatformUtil/ProcessRuntimeUtility.h"

#include <dlfcn.h>

__attribute__((constructor)) static void ctor() {
  DEBUG_LOG("================================");
  DEBUG_LOG("Dobby");
  DEBUG_LOG("dobby in debug log mode, disable with cmake flag \"-DDOBBY_DEBUG=OFF\"");
  DEBUG_LOG("================================");
}

PUBLIC const char *DobbyGetVersion() {
  return "1.0.0-modern";
}

PUBLIC void *DobbySymbolResolver(const char *image_name, const char *symbol_name) {
  void *result = dlsym(RTLD_DEFAULT, symbol_name);
  if (result)
    return result;

  if (image_name) {
    RuntimeModule module = ProcessRuntimeUtility::GetProcessModule(image_name);
    if (module.load_address) {
      dobby::ElfSymbolResolver resolver(module.path);
      return resolver.Resolve(symbol_name, reinterpret_cast<uintptr_t>(module.load_address));
    }
  }

  auto &module_map = ProcessRuntimeUtility::GetProcessModuleMap();
  for (const auto &module : module_map) {
    if (module.load_address) {
      dobby::ElfSymbolResolver resolver(module.path);
      void *res = resolver.Resolve(symbol_name, reinterpret_cast<uintptr_t>(module.load_address));
      if (res)
        return res;
    }
  }
  return nullptr;
}

PUBLIC DobbyStatus DobbyInitializePlugins(const char *config_path) {
  auto result = dobby::DobbyInitializePluginsFromConfig(config_path);
  if (!result) {
    return kDobbyFailed;
  }
  return kDobbySuccess;
}

PUBLIC DobbyStatus DobbyImportTableReplace(char *image_name, char *symbol_name, dobby_func_t fake_func,
                                   dobby_func_t *orig_func) {
  if (image_name) {
    RuntimeModule module = ProcessRuntimeUtility::GetProcessModule(image_name);
    if (module.load_address) {
      dobby::ImportTableResolver resolver(module.path);
      return resolver.Replace(symbol_name, fake_func, (void **)orig_func, reinterpret_cast<uintptr_t>(module.load_address));
    }
  }

  auto &module_map = ProcessRuntimeUtility::GetProcessModuleMap();
  for (const auto &module : module_map) {
    if (module.load_address) {
      dobby::ImportTableResolver resolver(module.path);
      DobbyStatus status = resolver.Replace(symbol_name, fake_func, (void **)orig_func, reinterpret_cast<uintptr_t>(module.load_address));
      if (status == kDobbySuccess)
        return kDobbySuccess;
    }
  }

  return kDobbyFailed;
}

#include <thread>
#include <chrono>

PUBLIC DobbyStatus DobbyDestroy(void *address) {
#if defined(TARGET_ARCH_ARM)
  if ((addr_t)address % 2) {
    address = (void *)((addr_t)address - 1);
  }
#endif

  auto entry_opt = Interceptor::SharedInstance()->find((addr_t)address);
  if (entry_opt) {
    auto entry = *entry_opt;
    uint8_t *buffer = entry->origin_insns;
    uint32_t buffer_size = entry->origin_insn_size;
    DobbyCodePatch(address, buffer, buffer_size);
    
    // Wait for a short period to ensure threads already in the trampoline can finish.
    // This is a simple quiescent state wait.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    Interceptor::SharedInstance()->remove((addr_t)address);
    return kDobbySuccess;
  }

  return kDobbyFailed;
}

PUBLIC DobbyStatus DobbyTransactionBegin() {
  Interceptor::SharedInstance()->transactionBegin();
  return kDobbySuccess;
}

PUBLIC DobbyStatus DobbyTransactionCommit() {
  return Interceptor::SharedInstance()->transactionCommit();
}
