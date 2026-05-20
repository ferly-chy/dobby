#if defined(ENABLE_CLOSURE_BRIDGE_TEMPLATE)

.align 4

#if !defined(ENABLE_CLOSURE_TRAMPOLINE_CARRY_OBJECT_PTR)

#define OFFSETOF_ClourseTrampolineEntry_carry_data 8
#define OFFSETOF_ClourseTrampolineEntry_carry_handler 0
    .globl closure_trampoline_template
    closure_trampoline_template:
        ldr x17, ClourseTrampolineEntryPtr
        ldr x16, [x17, #OFFSETOF_ClourseTrampolineEntry_carry_data]
        ldr x17, [x17, #OFFSETOF_ClourseTrampolineEntry_carry_handler]
        br x17
    ClourseTrampolineEntryPtr:
        .long 0
        .long 0

#else

    .globl closure_trampoline_template
    closure_trampoline_template:
        ldr x16, carry_data
        ldr x17, carry_handler
        br x17
    carry_data:
        .long 0
        .long 0
    carry_handler:
        .long 0
        .long 0

#endif

#endif
