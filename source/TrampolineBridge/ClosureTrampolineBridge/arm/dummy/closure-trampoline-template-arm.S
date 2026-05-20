#if defined(ENABLE_CLOSURE_BRIDGE_TEMPLATE)

.align 4

#if !defined(ENABLE_CLOSURE_TRAMPOLINE_CARRY_OBJECT_PTR)

#define OFFSETOF_ClourseTrampolineEntry_carry_data 4
#define OFFSETOF_ClourseTrampolineEntry_carry_handler 0
    .globl closure_trampoline_template
    closure_trampoline_template:
        ldr r12, ClourseTrampolineEntryPtr
        ldr pc, [r12, #0]
    ClourseTrampolineEntryPtr:
        .long 0

#else

    .globl closure_trampoline_template
    closure_trampoline_template:
        ldr r12, carry_data
        ldr pc, carry_handler
    carry_data:
        .long 0
    carry_handler:
        .long 0
#endif

#endif
