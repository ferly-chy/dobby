#pragma once

#include <stdint.h>

#ifndef PAC_KIT
#define PAC_KIT

static inline void *pac_strip(void *addr) {
  return addr;
}

static inline void *pac_sign(void *addr) {
  return addr;
}

static inline void *pac_strip_and_sign(void *addr) {
  return addr;
}

#endif
