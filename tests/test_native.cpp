#include "dobby.h"

#include <assert.h>
#include <stdio.h>

#define LOG(fmt, ...) printf("[test_native] " fmt "\n", ##__VA_ARGS__)

__attribute__((noinline)) static int target_add(int a, int b) {
  volatile int result = a + b;
  return result;
}

static bool g_instrument_called = false;

static void instrument_callback(void *address, DobbyRegisterContext *ctx) {
  (void)address;
#if defined(__arm64__) || defined(__aarch64__)
  LOG("target_add args: %llu, %llu",
      (unsigned long long)ctx->general.regs.x0,
      (unsigned long long)ctx->general.regs.x1);
#elif defined(__arm__)
  LOG("target_add args: %u, %u", ctx->general.regs.r0, ctx->general.regs.r1);
#elif defined(__x86_64__)
  LOG("target_add args: %llu, %llu",
      (unsigned long long)ctx->general.regs.rdi,
      (unsigned long long)ctx->general.regs.rsi);
#elif defined(__i386__)
  LOG("target_add instrumentation triggered");
#else
  LOG("target_add instrumentation triggered");
#endif
  g_instrument_called = true;
}

int main() {
  LOG("installing instrumentation");

  DobbyStatus status = DobbyInstrument((void *)target_add, instrument_callback);
  assert(status == kDobbySuccess);

  int result = target_add(7, 5);
  assert(result == 12);
  assert(g_instrument_called);

  status = DobbyDestroy((void *)target_add);
  assert(status == kDobbySuccess);

  LOG("test passed");
  return 0;
}
