#pragma once

#include <type_traits>

#include "dobby/dobby_internal.h"

#ifdef ENABLE_CLOSURE_TRAMPOLINE_TEMPLATE
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
void closure_trampoline_template();
void closure_bridge_template();
#ifdef __cplusplus
}
#endif //__cplusplus
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef void (*ClosureCarryHandler)(void *carry_data, DobbyRegisterContext *ctx);

typedef enum {
  kClosureCarryKindInvalid = 0,
  kClosureCarryKindRouteEntry,
} ClosureCarryKind;

typedef struct {
  void *address;
  int size;
  ClosureCarryHandler carry_handler;
  void *carry_data;
  ClosureCarryKind carry_kind;
} ClosureTrampolineEntry;

asm_func_t get_closure_bridge();

#ifdef __cplusplus
}
#endif //__cplusplus

class ClosureTrampoline {
public:
  template <typename CarryData>
  static ClosureTrampolineEntry *CreateClosureTrampoline(CarryData *carry_data, ClosureCarryHandler carry_handler,
                                                         ClosureCarryKind carry_kind) {
    static_assert(!std::is_function_v<CarryData>, "Carry data must be an object pointer");
    return CreateClosureTrampolineImpl(static_cast<void *>(carry_data), carry_handler, carry_kind);
  }

private:
  static ClosureTrampolineEntry *CreateClosureTrampolineImpl(void *carry_data, ClosureCarryHandler carry_handler,
                                                             ClosureCarryKind carry_kind);
};
