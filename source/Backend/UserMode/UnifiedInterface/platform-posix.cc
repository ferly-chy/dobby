#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "logging/logging.h"
#include "base/check_logging.h"
#include "PlatformUnifiedInterface/platform.h"

#if defined(__ANDROID__) && !defined(ANDROID_LOG_STDOUT)
#define ANDROID_LOG_TAG "Dobby"
#include <android/log.h>
#endif

#include <string.h>

const int kMmapFd = -1;
const int kMmapFdOffset = 0;

using namespace base;

typedef struct thread_handle_t {
  pthread_t thread;
} thread_handle_t;

void ThreadInterface::SetName(const char *name) {
  pthread_setname_np(pthread_self(), name);
}

int ThreadInterface::CurrentId() {
  return syscall(__NR_gettid);
}

static void *thread_handler_wrapper(void *ctx) {
  ThreadInterface::Delegate *d = (ThreadInterface::Delegate *)ctx;
  d->ThreadMain();
  return nullptr;
}

bool ThreadInterface::Create(ThreadInterface::Delegate *delegate, ThreadHandle *handle) {
  thread_handle_t *handle_impl = new thread_handle_t;

  int err = 0;
  err = pthread_create(&(handle_impl->thread), nullptr, thread_handler_wrapper, delegate);
  if (err != 0) {
    ERROR_LOG("pthread create failed");
    return false;
  }
  return true;
}

OSThread::OSThread(const char *name) {
  strncpy(name_, name, sizeof(name_) - 1);
}

bool OSThread::Start() {
  if (ThreadInterface::Create(this, &handle_) == false) {
    return false;
  }
  return true;
}

static int GetProtectionFromMemoryPermission(MemoryPermission access) {
  switch (access) {
  case MemoryPermission::kNoAccess:
    return PROT_NONE;
  case MemoryPermission::kRead:
    return PROT_READ;
  case MemoryPermission::kReadWrite:
    return PROT_READ | PROT_WRITE;
  case MemoryPermission::kReadWriteExecute:
    return PROT_READ | PROT_WRITE | PROT_EXEC;
  case MemoryPermission::kReadExecute:
    return PROT_READ | PROT_EXEC;
  }
  UNREACHABLE();
}

int OSMemory::PageSize() {
  return static_cast<int>(sysconf(_SC_PAGESIZE));
}

void *OSMemory::Allocate(size_t size, MemoryPermission access) {
  return OSMemory::Allocate(size, access, nullptr);
}

void *OSMemory::Allocate(size_t size, MemoryPermission access, void *fixed_address) {
  int prot = GetProtectionFromMemoryPermission(access);

  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  if (fixed_address != nullptr) {
    flags = flags | MAP_FIXED;
  }
  void *result = mmap(fixed_address, size, prot, flags, kMmapFd, kMmapFdOffset);
  if (result == MAP_FAILED)
    return nullptr;

  return result;
}

bool OSMemory::Free(void *address, size_t size) {
  DCHECK_EQ(0, reinterpret_cast<uintptr_t>(address) % PageSize());
  DCHECK_EQ(0, size % PageSize());

  return munmap(address, size) == 0;
}

bool OSMemory::Release(void *address, size_t size) {
  DCHECK_EQ(0, reinterpret_cast<uintptr_t>(address) % PageSize());
  DCHECK_EQ(0, size % PageSize());

  return munmap(address, size) == 0;
}

bool OSMemory::SetPermission(void *address, size_t size, MemoryPermission access) {
  DCHECK_EQ(0, reinterpret_cast<uintptr_t>(address) % PageSize());
  DCHECK_EQ(0, size % PageSize());

  int prot = GetProtectionFromMemoryPermission(access);
  int ret = mprotect(address, size, prot);
  if (ret) {
    ERROR_LOG("OSMemory::SetPermission: %s\n", ((const char *)strerror(errno)));
  }

  return ret == 0;
}

void OSPrint::Print(const char *format, ...) {
  va_list args;
  va_start(args, format);
  VPrint(format, args);
  va_end(args);
}

void OSPrint::VPrint(const char *format, va_list args) {
#if defined(__ANDROID__) && !defined(ANDROID_LOG_STDOUT)
  __android_log_vprint(ANDROID_LOG_INFO, ANDROID_LOG_TAG, format, args);
#else
  vprintf(format, args);
#endif
}
