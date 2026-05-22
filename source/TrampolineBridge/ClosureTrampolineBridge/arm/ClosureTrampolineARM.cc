#include "platform_detect_macro.h"
#if defined(TARGET_ARCH_ARM)

#include "dobby/dobby_internal.h"

#include "core/assembler/assembler-arm.h"

#include "TrampolineBridge/ClosureTrampolineBridge/ClosureTrampoline.h"

using namespace zz;
using namespace zz::arm;

ClosureTrampolineEntry *ClosureTrampoline::CreateClosureTrampolineImpl(void *carry_data, void *carry_handler,
                                                                       ClosureCarryKind carry_kind) {
  ClosureTrampolineEntry *tramp_entry = nullptr;
  tramp_entry = new ClosureTrampolineEntry;

#ifdef ENABLE_CLOSURE_TRAMPOLINE_TEMPLATE
  // Closure trampoline template is not implemented for ARM yet.
  // We prefer the dynamic assembler-based approach below.
  return nullptr;
#else
  // use assembler and codegen modules instead of template_code
  #define _ turbo_assembler_.
  TurboAssembler turbo_assembler_(0);

  AssemblerPseudoLabel entry_label(0);
  AssemblerPseudoLabel forward_bridge_label(0);

  _ Ldr(r12, &entry_label);
  _ Ldr(pc, &forward_bridge_label);
  _ PseudoBind(&entry_label);
  _ EmitAddress((uint32_t)(uintptr_t)tramp_entry);
  _ PseudoBind(&forward_bridge_label);
  _ EmitAddress((uint32_t)(uintptr_t)get_closure_bridge());

  auto closure_tramp = AssemblyCodeBuilder::FinalizeFromTurboAssembler(&turbo_assembler_);
  tramp_entry->address = (void *)closure_tramp->addr;
  tramp_entry->size = closure_tramp->size;
  tramp_entry->carry_kind = carry_kind;
  if (carry_kind == kClosureCarryKindRouteEntry) {
    tramp_entry->metadata.route.entry = (InterceptEntry *)carry_data;
    tramp_entry->metadata.route.handler = (void (*)(InterceptEntry *, DobbyRegisterContext *))carry_handler;
  } else {
    tramp_entry->metadata.user.data = carry_data;
    tramp_entry->metadata.user.handler = (void (*)(void *, DobbyRegisterContext *))carry_handler;
  }

  delete closure_tramp;

  return tramp_entry;
#endif
}

#endif