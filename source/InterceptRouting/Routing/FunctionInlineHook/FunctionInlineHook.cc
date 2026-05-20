#include "dobby/dobby_internal.h"

#include "Interceptor.h"
#include "InterceptRouting/Routing/FunctionInlineHook/FunctionInlineHookRouting.h"

PUBLIC DobbyStatus DobbyHook(void *address, dobby_func_t replace_func, dobby_func_t *origin_func) {
  if (!address) {
    ERROR_LOG("function address is 0x0");
    return kDobbyFailed;
  }

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
  }

  routing->Commit();

  Interceptor::SharedInstance()->add(entry);

  return kDobbySuccess;
}
