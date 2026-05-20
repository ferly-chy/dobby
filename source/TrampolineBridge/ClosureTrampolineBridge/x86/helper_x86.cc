#include "platform_detect_macro.h"
#if defined(TARGET_ARCH_IA32)

#include "dobby/dobby_internal.h"

void set_routing_bridge_next_hop(DobbyRegisterContext *ctx, addr_t address) {
  addr_t esp = ctx->esp;

  addr_t entry_placeholder_stack_addr = esp - 4;
  *(addr_t *)entry_placeholder_stack_addr = address;
}

#endif
