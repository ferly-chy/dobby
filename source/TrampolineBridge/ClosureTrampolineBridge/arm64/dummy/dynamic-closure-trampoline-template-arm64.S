#if defined(ENABLE_CLOSURE_BRIDGE_TEMPLATE)

#define PAGE_MAX_SIZE 4096
#define PAGE_MAX_SHIFT 14

.align PAGE_MAX_SHIFT

.globl dynamic_closure_trampoline_table_page
dynamic_closure_trampoline_table_page:
    .rept (PAGE_MAX_SIZE - 4 * 4) / 8
    adr x16, #0
    b dynamic_closure_trampoline_forward
    .endr

    dynamic_closure_trampoline_forward:
    sub x16, x16, #0x4000
    ldr x16, [x16, #0]
    ldr x17, [x16, #0]
    br x17

#endif
