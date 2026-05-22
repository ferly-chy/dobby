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

typedef enum {
  kClosureCarryKindInvalid = 0,
  kClosureCarryKindRouteEntry,
  kClosureCarryKindUserDefined,
} ClosureCarryKind;

typedef struct ClosureTrampolineEntry_ {
  void *address;
  int size;

  ClosureCarryKind carry_kind;
  union {
    struct {
      struct InterceptEntry *entry;
      void (*handler)(struct InterceptEntry *entry, DobbyRegisterContext *ctx);
    } route;
    struct {
      void *data;
      void (*handler)(void *data, DobbyRegisterContext *ctx);
    } user;
  } metadata;
} ClosureTrampolineEntry;

asm_func_t get_closure_bridge();

#ifdef __cplusplus
}
#endif //__cplusplus

class ClosureTrampoline {
public:
  static ClosureTrampolineEntry *CreateClosureTrampoline(struct InterceptEntry *entry,
                                                         void (*handler)(struct InterceptEntry *,
                                                                         DobbyRegisterContext *)) {
    return CreateClosureTrampolineImpl((void *)entry, (void *)handler, kClosureCarryKindRouteEntry);
  }

  static ClosureTrampolineEntry *CreateClosureTrampoline(void *data,
                                                         void (*handler)(void *, DobbyRegisterContext *)) {
    return CreateClosureTrampolineImpl(data, (void *)handler, kClosureCarryKindUserDefined);
  }

private:
  static ClosureTrampolineEntry *CreateClosureTrampolineImpl(void *carry_data, void *carry_handler,
                                                             ClosureCarryKind carry_kind);
};
