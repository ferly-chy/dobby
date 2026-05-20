#include "platform_detect_macro.h"
#if defined(TARGET_ARCH_ARM)

#include "dobby/dobby_internal.h"

void set_routing_bridge_next_hop(DobbyRegisterContext *ctx, addr_t address) {
  ctx->general.regs.r12 = address;
}

#endif
