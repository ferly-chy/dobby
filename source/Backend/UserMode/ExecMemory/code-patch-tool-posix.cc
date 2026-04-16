
#include "dobby/dobby_internal.h"
#include "MultiThreadSupport/ThreadFreezer.h"

#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#if !defined(__APPLE__)
PUBLIC int DobbyCodePatch(void *address, uint8_t *buffer, uint32_t buffer_size) {
#if defined(__ANDROID__) || defined(__linux__)
  if (!address || !buffer || buffer_size == 0) {
    return kDobbyErrorInvalidArgument;
  }

  // Use RAII ThreadFreezer to stop all threads during patching
  dobby::ThreadFreezer freezer;
  if (!freezer.FreezeAll()) {
    ERROR_LOG("DobbyCodePatch: failed to freeze threads");
    // Continue anyway as a best effort
  }

  int page_size = (int)sysconf(_SC_PAGESIZE);
  uintptr_t patch_page = ALIGN_FLOOR(address, page_size);
  uintptr_t patch_end = (uintptr_t)address + buffer_size;
  uintptr_t patch_end_page_start = ALIGN_FLOOR(patch_end - 1, page_size);
  size_t patch_len = (patch_end_page_start - patch_page) + page_size;

  // Try to set RWX (standard for hooking)
  if (mprotect((void *)patch_page, patch_len, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
    // Fallback to RW if RWX is not allowed (SELinux / modern Android)
    if (mprotect((void *)patch_page, patch_len, PROT_READ | PROT_WRITE) != 0) {
      ERROR_LOG("DobbyCodePatch: mprotect RW failed: %s", strerror(errno));
      return kDobbyErrorMemoryProtection;
    }
  }

  // Patch the memory
  memcpy(address, buffer, buffer_size);

  // Restore to RX as a reasonable default for code pages.
  // Note: We don't know the ORIGINAL permissions here without parsing /proc/self/maps,
  // but RX is the standard expectation for executable code.
  mprotect((void *)patch_page, patch_len, PROT_READ | PROT_EXEC);

  // Flush instruction cache
  ClearCache(address, (void *)((uintptr_t)address + buffer_size));

  return kDobbySuccess;
#else
  return kDobbySuccess;
#endif
}
#endif