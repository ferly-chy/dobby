
#include "dobby/dobby_internal.h"

#include "logging/logging.h"

#include "intercept_routing_handler.h"

#include "function-wrapper.h"

#include "MultiThreadSupport/ThreadSupport.h"

#include "TrampolineBridge/ClosureTrampolineBridge/common_bridge_handler.h"

void pre_call_forward_handler(DobbyRegisterContext *ctx, InterceptEntry *entry) {
  FunctionWrapperRouting *routing = (FunctionWrapperRouting *)entry->routing;

  StackFrame *stackframe = new StackFrame();
  // create stack frame as common variable between pre_call and post_call
  ThreadSupport::PushStackFrame(stackframe);

  // run the `pre_call` before execute origin function which has been relocated(fixed)
  if (routing->pre_call) {
    PreCallTy pre_call = routing->pre_call;
    InterceptEntry callback_entry = *entry;
    // run the pre_call with the power of accessing all registers
    (*pre_call)(ctx, (const InterceptEntry *)&callback_entry);
  }

  // save the origin ret address, and use in `post_call_forword_handler`
  stackframe->orig_ret = get_func_ret_address(ctx);

  // set the prologue bridge next hop address with the patched instructions has been relocated
  set_routing_bridge_next_hop(ctx, entry->relocated_origin_function);

  // replace the function ret address with our epilogue_routing_dispatch
  set_func_ret_address(ctx, entry->epilogue_dispatch_bridge);
}

void post_call_forward_handler(DobbyRegisterContext *ctx, InterceptEntry *entry) {
  FunctionWrapperRouting *routing = (FunctionWrapperRouting *)entry->routing;

  // pop stack frame as common variable between pre_call and post_call
  StackFrame *stackframe = ThreadSupport::PopStackFrame();

  // run the `post_call`, and access all the register value, as the origin function done,
  if (routing->post_call) {
    PostCallTy post_call = routing->post_call;
    InterceptEntry callback_entry = *entry;

    // run the post_call with the power of accessing all registers
    (*post_call)(ctx, (const InterceptEntry *)&callback_entry);
  }

  // set epilogue bridge next hop address with origin ret address, restore the call.
  set_routing_bridge_next_hop(ctx, stackframe->orig_ret);
}

// run the user handler **before run the origin-instructions(which have been relocated)**
void prologue_routing_dispatch(void *carry_data, DobbyRegisterContext *ctx) {
  DEBUG_LOG("Catch prologue dispatch");
  auto *entry = static_cast<InterceptEntry *>(carry_data);
  if (entry == nullptr) {
    return;
  }
  pre_call_forward_handler(ctx, entry);
  return;
}

// run the user handler **before the function return** by replace the lr register
void epilogue_routing_dispatch(void *carry_data, DobbyRegisterContext *ctx) {
  DEBUG_LOG("Catch epilogue dispatch");
  auto *entry = static_cast<InterceptEntry *>(carry_data);
  if (entry == nullptr) {
    return;
  }
  post_call_forward_handler(ctx, entry);
  return;
}