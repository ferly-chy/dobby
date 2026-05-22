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

  auto entry = std::make_unique<InterceptEntry>(kFunctionInlineHook, (addr_t)address);
  auto entry_ptr = entry.get();

  auto routing = std::make_unique<FunctionInlineHookRouting>(entry_ptr, replace_func);
  routing->Prepare();
  routing->DispatchRouting();

  // set origin func entry with as relocated instructions
  if (origin_func) {
    *origin_func = (dobby_func_t)entry_ptr->relocated_addr;
  }

  routing->Commit();

  entry->routing = std::move(routing);
  Interceptor::SharedInstance()->add(std::move(entry));

  return kDobbySuccess;
}
