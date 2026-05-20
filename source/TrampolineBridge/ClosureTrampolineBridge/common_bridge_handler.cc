#include "TrampolineBridge/ClosureTrampolineBridge/common_bridge_handler.h"

#include "dobby/dobby_internal.h"

#include "InterceptEntry.h"

#include "TrampolineBridge/ClosureTrampolineBridge/ClosureTrampoline.h"

void common_closure_bridge_handler(DobbyRegisterContext *ctx, ClosureTrampolineEntry *closure_trampoline_entry) {
  if (closure_trampoline_entry == nullptr || closure_trampoline_entry->carry_kind != kClosureCarryKindRouteEntry) {
    return;
  }

  auto *entry = static_cast<InterceptEntry *>(closure_trampoline_entry->carry_data);
  if (entry == nullptr) {
    return;
  }

  if (closure_trampoline_entry->carry_handler != nullptr) {
    closure_trampoline_entry->carry_handler(closure_trampoline_entry->carry_data, ctx);
    return;
  }

  if (entry->type == kFunctionInlineHook) {
    // nothing to do
  }

  set_routing_bridge_next_hop(ctx, entry->relocated_addr);
}
