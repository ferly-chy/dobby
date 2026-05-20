// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/base/platform/semaphore.h"

#include <errno.h>
#include <semaphore.h>

#include "src/base/logging.h"
#include "src/base/platform/elapsed-timer.h"
#include "src/base/platform/time.h"
#include "logging/logging.h"

namespace v8 {
namespace base {

Semaphore::Semaphore(int count) {
  DCHECK_GE(count, 0);
  int result = sem_init(&native_handle_, 0, count);
  DCHECK_EQ(0, result);
}

Semaphore::~Semaphore() {
  int result = sem_destroy(&native_handle_);
  DCHECK_EQ(0, result);
}

void Semaphore::Signal() {
  int result = sem_post(&native_handle_);
  if (result != 0) {
    ERROR_LOG("Error when signaling semaphore, errno: %d", errno);
  }
}

void Semaphore::Wait() {
  while (true) {
    int result = sem_wait(&native_handle_);
    if (result == 0)
      return;
    DCHECK_EQ(-1, result);
    DCHECK_EQ(EINTR, errno);
  }
}

bool Semaphore::WaitFor(const TimeDelta &rel_time) {
  const Time time = Time::NowFromSystemTime() + rel_time;
  const struct timespec ts = time.ToTimespec();

  while (true) {
    int result = sem_timedwait(&native_handle_, &ts);
    if (result == 0)
      return true;
    if (result == -1 && errno == ETIMEDOUT) {
      return false;
    }
    DCHECK_EQ(-1, result);
    DCHECK_EQ(EINTR, errno);
  }
}

} // namespace base
} // namespace v8
