#include "TrampolineBridge/ClosureTrampolineBridge/common_bridge_handler.h"

#include "dobby/dobby_internal.h"

#include "InterceptEntry.h"

#include "TrampolineBridge/ClosureTrampolineBridge/ClosureTrampoline.h"

void common_closure_bridge_handler(DobbyRegisterContext *ctx, ClosureTrampolineEntry *closure_trampoline_entry) {
  InterceptEntry *entry = (InterceptEntry *)closure_trampoline_entry->carry_data;

  if (entry->type == kFunctionInlineHook) {
    // nothing to do
  } else if (entry->type == kInstructionInstrument) {
    dobby_instrument_callback_t pre_handler = (dobby_instrument_callback_t)closure_trampoline_entry->carry_handler;
    if (pre_handler) {
      pre_handler((void *)entry->patched_addr, ctx);
    }
  }

  set_routing_bridge_next_hop(ctx, entry->relocated_addr);
}
