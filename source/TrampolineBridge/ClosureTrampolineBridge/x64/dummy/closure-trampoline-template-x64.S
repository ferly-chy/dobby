#if defined(ENABLE_CLOSURE_BRIDGE_TEMPLATE)

.align 4

    .globl closure_trampoline_template
    closure_trampoline_template:
        push [rip + 6 + 6]
        jmp [rip + 6 + 8]
    carry_data:
        .long 0
        .long 0
    carry_handler:
        .long 0
        .long 0

#endif
