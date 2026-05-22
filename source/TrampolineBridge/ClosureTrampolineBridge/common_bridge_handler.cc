#include "TrampolineBridge/ClosureTrampolineBridge/common_bridge_handler.h"

#include "dobby/dobby_internal.h"

#include "InterceptEntry.h"

#include "TrampolineBridge/ClosureTrampolineBridge/ClosureTrampoline.h"

void common_closure_bridge_handler(DobbyRegisterContext *ctx, ClosureTrampolineEntry *closure_trampoline_entry) {
  if (closure_trampoline_entry == nullptr) {
    return;
  }

  if (closure_trampoline_entry->carry_kind == kClosureCarryKindRouteEntry) {
    auto *entry = closure_trampoline_entry->metadata.route.entry;
    if (entry == nullptr) {
      return;
    }

    if (closure_trampoline_entry->metadata.route.handler != nullptr) {
      closure_trampoline_entry->metadata.route.handler(entry, ctx);
      return;
    }

    if (entry->type == kFunctionInlineHook) {
      // nothing to do
    }

    set_routing_bridge_next_hop(ctx, entry->relocated_addr);
  } else if (closure_trampoline_entry->carry_kind == kClosureCarryKindUserDefined) {
    if (closure_trampoline_entry->metadata.user.handler != nullptr) {
      closure_trampoline_entry->metadata.user.handler(closure_trampoline_entry->metadata.user.data, ctx);
    }
  }
}
