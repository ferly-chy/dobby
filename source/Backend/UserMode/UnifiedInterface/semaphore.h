// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_BASE_PLATFORM_SEMAPHORE_H_
#define V8_BASE_PLATFORM_SEMAPHORE_H_

#include "src/base/base-export.h"
#include "src/base/lazy-instance.h"
#include <semaphore.h>

namespace v8 {
namespace base {

class TimeDelta;

class V8_BASE_EXPORT Semaphore final {
public:
  explicit Semaphore(int count);
  ~Semaphore();

  void Signal();

  void Wait();

  bool WaitFor(const TimeDelta &rel_time);

  using NativeHandle = sem_t;

  NativeHandle &native_handle() {
    return native_handle_;
  }
  const NativeHandle &native_handle() const {
    return native_handle_;
  }

private:
  NativeHandle native_handle_;

  DISALLOW_COPY_AND_ASSIGN(Semaphore);
};

template <int N> struct CreateSemaphoreTrait {
  static Semaphore *Create() {
    return new Semaphore(N);
  }
};

template <int N> struct LazySemaphore {
  using type = typename LazyDynamicInstance<Semaphore, CreateSemaphoreTrait<N>, ThreadSafeInitOnceTrait>::type;
};

#define LAZY_SEMAPHORE_INITIALIZER LAZY_DYNAMIC_INSTANCE_INITIALIZER

} // namespace base
} // namespace v8

#endif // V8_BASE_PLATFORM_SEMAPHORE_H_
