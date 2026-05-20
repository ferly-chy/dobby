#include "Thread/PlatformThread.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

using namespace zz;

int OSThread::GetCurrentProcessId() {
  return static_cast<int>(getpid());
}

int OSThread::GetCurrentThreadId() {
  return static_cast<int>(gettid());
}

static OSThread::LocalStorageKey PthreadKeyToLocalKey(pthread_key_t pthread_key) {
  return static_cast<OSThread::LocalStorageKey>(pthread_key);
}

static pthread_key_t LocalKeyToPthreadKey(OSThread::LocalStorageKey local_key) {
  return static_cast<pthread_key_t>(local_key);
}

OSThread::LocalStorageKey OSThread::CreateThreadLocalKey() {
  pthread_key_t key;
  int result = pthread_key_create(&key, nullptr);
  DCHECK_EQ(0, result);
  LocalStorageKey local_key = PthreadKeyToLocalKey(key);
  return local_key;
}

void OSThread::DeleteThreadLocalKey(LocalStorageKey key) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  int result = pthread_key_delete(pthread_key);
  DCHECK_EQ(0, result);
}

void *OSThread::GetThreadLocal(LocalStorageKey key) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  return pthread_getspecific(pthread_key);
}

void OSThread::SetThreadLocal(LocalStorageKey key, void *value) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  int result = pthread_setspecific(pthread_key, value);
  DCHECK_EQ(0, result);
}
