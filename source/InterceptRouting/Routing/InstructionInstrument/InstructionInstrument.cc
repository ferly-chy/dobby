#include "dobby/dobby_internal.h"

#include "Interceptor.h"
#include "InterceptRouting/InterceptRouting.h"
#include "InterceptRouting/Routing/InstructionInstrument/InstructionInstrumentRouting.h"

PUBLIC DobbyStatus DobbyInstrument(void *address, dobby_instrument_callback_t pre_handler) {
  if (!address) {
    ERROR_LOG("address is 0x0.\n");
    return kDobbyFailed;
  }

  DEBUG_LOG("\n\n----- [DobbyInstrument:%p] -----", address);


  auto entry_opt = Interceptor::SharedInstance()->find((addr_t)address);
  if (entry_opt) {
    ERROR_LOG("%p already been instrumented.", address);
    return kDobbyFailed;
  }

  auto entry = std::make_unique<InterceptEntry>(kInstructionInstrument, (addr_t)address);
  auto entry_ptr = entry.get();

  auto routing = std::make_unique<InstructionInstrumentRouting>(entry_ptr, pre_handler, nullptr);
  routing->Prepare();
  routing->DispatchRouting();
  routing->Commit();

  entry->routing = std::move(routing);
  Interceptor::SharedInstance()->add(std::move(entry));

  return kDobbySuccess;
}
