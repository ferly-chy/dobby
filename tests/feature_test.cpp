#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "dobby.h"

// --- Mock Functions for Hooking ---

__attribute__((noinline)) int target_func(int a, int b) {
    volatile int res = a + b;
    printf("  [Target] Eksekusi fungsi asli: %d + %d = %d\n", a, b, res);
    return res;
}

int (*orig_target_func)(int a, int b);

int fake_target_func(int a, int b) {
    printf("  [Fake] Intersepsi aktif! Input diterima: %d, %d\n", a, b);
    int manipulated_res = (a + b) * 10;
    printf("  [Fake] Mengembalikan nilai manipulasi: %d\n", manipulated_res);
    return manipulated_res;
}

// --- Mock Handler for Instrumentation ---

void instrument_handler(void *address, DobbyRegisterContext *ctx) {
    printf("  [Instrument] Handler dipicu pada alamat: %p\n", address);
#if defined(__arm64__) || defined(__aarch64__)
    unsigned long x0 = (unsigned long)ctx->general.regs.x0;
    unsigned long x1 = (unsigned long)ctx->general.regs.x1;
    printf("  [Instrument] Register State -> X0 (Arg 1): %ld, X1 (Arg 2): %ld\n", x0, x1);
#elif defined(__arm__)
    uint32_t r0 = ctx->general.regs.r0;
    uint32_t r1 = ctx->general.regs.r1;
    printf("  [Instrument] Register State -> R0 (Arg 1): %u, R1 (Arg 2): %u\n", r0, r1);
#endif
}

// --- Test Suites ---

void test_dobby_hook() {
    printf("\n>>> TAHAP 1: DobbyHook (Inline Hooking)\n");
    
    printf("1. Memanggil fungsi sebelum di-hook:\n");
    int val = target_func(5, 5);
    printf("   Hasil asli: %d\n", val);
    assert(val == 10);

    printf("2. Memasang hook (target_func -> fake_target_func)...\n");
    DobbyStatus status = DobbyHook((void *)target_func, (dobby_func_t)fake_target_func, (dobby_func_t *)&orig_target_func);
    assert(status == kDobbySuccess);
    printf("   Hook terpasang: [kDobbySuccess]\n");

    printf("3. Memanggil fungsi setelah di-hook:\n");
    int hooked_val = target_func(5, 5);
    printf("   Hasil akhir di caller: %d (Ekspektasi: 100)\n", hooked_val);
    assert(hooked_val == 100);

    printf("4. Melepas hook menggunakan DobbyDestroy...\n");
    status = DobbyDestroy((void *)target_func);
    assert(status == kDobbySuccess);
    
    printf("5. Memastikan fungsi kembali normal:\n");
    int final_val = target_func(5, 5);
    printf("   Hasil setelah restorasi: %d\n", final_val);
    assert(final_val == 10);
    
    printf("[PASS] DobbyHook berfungsi dengan benar.\n");
}

void test_dobby_instrument() {
    printf("\n>>> TAHAP 2: DobbyInstrument (Dynamic Instrumentation)\n");
    
    printf("1. Memasang instrumentasi pada target_func...\n");
    DobbyStatus status = DobbyInstrument((void *)target_func, instrument_handler);
    assert(status == kDobbySuccess);
    printf("   Instrument terpasang: [kDobbySuccess]\n");

    printf("2. Memanggil fungsi (memicu handler & fungsi asli):\n");
    int val = target_func(13, 37);
    printf("   Fungsi tetap berjalan, hasil: %d\n", val);
    assert(val == 50);

    printf("3. Melepas instrumentasi...\n");
    status = DobbyDestroy((void *)target_func);
    assert(status == kDobbySuccess);
    
    printf("[PASS] DobbyInstrument berfungsi dengan benar.\n");
}

void test_dobby_code_patch() {
    printf("\n>>> TAHAP 3: DobbyCodePatch (Memory Patching)\n");
    
    const size_t sz = 8;
    uint8_t *mock_mem = (uint8_t *)malloc(sz);
    memset(mock_mem, 0xAA, sz); // Isi awal: [AA AA AA AA AA AA AA AA]
    
    printf("1. Data memori sebelum patch: ");
    for(size_t i=0; i<sz; i++) printf("%02X ", mock_mem[i]);
    printf("\n");

    uint8_t patch_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    printf("2. Menerapkan patch: [DE AD BE EF]\n");
    
    DobbyStatus status = DobbyCodePatch(mock_mem, patch_data, sizeof(patch_data));
    assert(status == kDobbySuccess);

    printf("3. Data memori setelah patch: ");
    for(size_t i=0; i<sz; i++) printf("%02X ", mock_mem[i]);
    printf("\n");

    assert(mock_mem[0] == 0xDE && mock_mem[3] == 0xEF);
    assert(mock_mem[4] == 0xAA); // Pastikan sisa memori tidak berubah
    
    free(mock_mem);
    printf("[PASS] DobbyCodePatch berfungsi dengan benar.\n");
}

int main() {
    printf("====================================================\n");
    printf("      DETAIL TEST SUITE: DOBBY CORE FEATURES        \n");
    printf("      Standards: C23 / C++26 | Maintainer: Gemini   \n");
    printf("====================================================\n");

    test_dobby_hook();
    test_dobby_instrument();
    test_dobby_code_patch();

    printf("\n====================================================\n");
    printf(" [SUCCESS] Seluruh verifikasi fitur Dobby SELESAI.  \n");
    printf("====================================================\n");

    return 0;
}
