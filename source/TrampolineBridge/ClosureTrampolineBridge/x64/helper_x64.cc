#include "platform_detect_macro.h"
#if defined(TARGET_ARCH_X64)

#include "dobby/dobby_internal.h"

void set_routing_bridge_next_hop(DobbyRegisterContext *ctx, addr_t address) {
  addr_t rsp = ctx->rsp;

  addr_t entry_placeholder_stack_addr = rsp - 8;
  *(addr_t *)entry_placeholder_stack_addr = address;
}

#endif
