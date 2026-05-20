#include "platform_detect_macro.h"
#if defined(TARGET_ARCH_X64)

#include "dobby/dobby_internal.h"

#include "core/assembler/assembler-x64.h"

#include "TrampolineBridge/ClosureTrampolineBridge/ClosureTrampoline.h"

using namespace zz;
using namespace zz::x64;

ClosureTrampolineEntry *ClosureTrampoline::CreateClosureTrampolineImpl(void *carry_data, ClosureCarryHandler carry_handler,
                                                                       ClosureCarryKind carry_kind) {
  ClosureTrampolineEntry *tramp_entry = nullptr;
  tramp_entry = new ClosureTrampolineEntry;

  auto tramp_size = 32;
  auto tramp_mem = MemoryAllocator::SharedAllocator()->allocateExecMemory(tramp_size);
  if (tramp_mem == nullptr) {
    return nullptr;
  }
#define _ turbo_assembler_.
#define __ turbo_assembler_.GetCodeBuffer()->
  TurboAssembler turbo_assembler_(0);

  uint8_t *push_rip_6 = (uint8_t *)"\xff\x35\x06\x00\x00\x00";
  uint8_t *jmp_rip_8 = (uint8_t *)"\xff\x25\x08\x00\x00\x00";

  __ EmitBuffer(push_rip_6, 6);
  __ EmitBuffer(jmp_rip_8, 6);
  __ Emit64((uint64_t)tramp_entry);
  __ Emit64((uint64_t)get_closure_bridge());

  tramp_entry->address = tramp_mem;
  tramp_entry->size = tramp_size;
  tramp_entry->carry_data = carry_data;
  tramp_entry->carry_handler = carry_handler;
  tramp_entry->carry_kind = carry_kind;

  auto closure_tramp_buffer = static_cast<CodeBufferBase *>(turbo_assembler_.GetCodeBuffer());
  auto buffer_span = closure_tramp_buffer->GetBuffer();
  DobbyCodePatch(tramp_mem, buffer_span.data(), (uint32_t)buffer_span.size());

  return tramp_entry;
}

#endif