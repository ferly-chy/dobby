//===-- clear_cache.c - Implement __clear_cache ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <assert.h>
#include <stddef.h>

// The compiler generates calls to __clear_cache() when creating
// trampoline functions on the stack for use with nested functions.
// It is expected to invalidate the instruction cache for the
// specified range.

void _clear_cache(void *start, void *end) {
#if __i386__ || __x86_64__ || defined(_M_IX86) || defined(_M_X64)
// Intel processors have a unified instruction and data cache
// so there is nothing to do
#elif defined(__arm__)
#define __ARM_NR_cacheflush 0x0f0002
  register int start_reg __asm("r0") = (int)(intptr_t)start;
  const register int end_reg __asm("r1") = (int)(intptr_t)end;
  const register int flags __asm("r2") = 0;
  const register int syscall_nr __asm("r7") = __ARM_NR_cacheflush;
  __asm __volatile("svc 0x0" : "=r"(start_reg) : "r"(syscall_nr), "r"(start_reg), "r"(end_reg), "r"(flags));
  assert(start_reg == 0 && "Cache flush syscall failed.");
#elif defined(__aarch64__)
  uint64_t xstart = (uint64_t)(uintptr_t)start;
  uint64_t xend = (uint64_t)(uintptr_t)end;

  // Get Cache Type Info.
  static uint64_t ctr_el0 = 0;
  if (ctr_el0 == 0)
    __asm __volatile("mrs %0, ctr_el0" : "=r"(ctr_el0));

  // The DC and IC instructions must use 64-bit registers so we don't use
  // uintptr_t in case this runs in an IPL32 environment.
  uint64_t addr;

  // If CTR_EL0.IDC is set, data cache cleaning to the point of unification
  // is not required for instruction to data coherence.
  if (((ctr_el0 >> 28) & 0x1) == 0x0) {
    const size_t dcache_line_size = 4 << ((ctr_el0 >> 16) & 15);
    for (addr = xstart & ~(dcache_line_size - 1); addr < xend; addr += dcache_line_size)
      __asm __volatile("dc cvau, %0" ::"r"(addr));
  }
  __asm __volatile("dsb ish");

  // If CTR_EL0.DIC is set, instruction cache invalidation to the point of
  // unification is not required for instruction to data coherence.
  if (((ctr_el0 >> 29) & 0x1) == 0x0) {
    const size_t icache_line_size = 4 << ((ctr_el0 >> 0) & 15);
    for (addr = xstart & ~(icache_line_size - 1); addr < xend; addr += icache_line_size)
      __asm __volatile("ic ivau, %0" ::"r"(addr));
    __asm __volatile("dsb ish");
  }
  __asm __volatile("isb sy");
#else
#error "Unsupported architecture for cache flushing"
#endif
}

void ClearCache(void *start, void *end) {
  return _clear_cache(start, end);
}
