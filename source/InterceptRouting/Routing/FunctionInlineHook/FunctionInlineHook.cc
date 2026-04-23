#include "dobby/dobby_internal.h"

#include "Interceptor.h"
#include "InterceptRouting/Routing/FunctionInlineHook/FunctionInlineHookRouting.h"

PUBLIC DobbyStatus DobbyHook(void *address, dobby_func_t replace_func, dobby_func_t *origin_func) {
  if (!address) {
    ERROR_LOG("function address is 0x0");
    return kDobbyFailed;
  }

#if defined(__APPLE__) && defined(__arm64__)
  address = pac_strip(address);
  replace_func = pac_strip(replace_func);
#endif

#if defined(ANDROID)
  void *page_align_address = (void *)ALIGN_FLOOR(address, OSMemory::PageSize());
  if (!OSMemory::SetPermission(page_align_address, OSMemory::PageSize(), kReadExecute)) {
    return kDobbyFailed;
  }
#endif

  DEBUG_LOG("----- [DobbyHook:%p] -----", address);

  // check if already register
  auto entry_opt = Interceptor::SharedInstance()->find((addr_t)address);
  if (entry_opt) {
    ERROR_LOG("%p already been hooked.", address);
    return kDobbyFailed;
  }

  auto entry = new InterceptEntry(kFunctionInlineHook, (addr_t)address);

  auto *routing = new FunctionInlineHookRouting(entry, replace_func);
  routing->Prepare();
  routing->DispatchRouting();

  // set origin func entry with as relocated instructions
  if (origin_func) {
    *origin_func = (dobby_func_t)entry->relocated_addr;
#if defined(__APPLE__) && defined(__arm64__)
    *origin_func = pac_sign(*origin_func);
#endif
  }

  routing->Commit();

  Interceptor::SharedInstance()->add(entry);

  return kDobbySuccess;
}
