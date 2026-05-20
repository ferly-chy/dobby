#pragma once

#include <assert.h>

#ifdef __cplusplus
#define DOBBY_ASSERT_NOT_NULL(ptr) assert((ptr) != nullptr)
#else
#define DOBBY_ASSERT_NOT_NULL(ptr) assert((ptr) != NULL)
#endif

#define ASSERT(expr) assert((expr))
#define DCHECK(expr) assert((expr))
#define DCHECK_EQ(a, b) assert((a) == (b))
#define DCHECK_GE(a, b) assert((a) >= (b))
#define DCHECK_NOT_NULL(ptr) DOBBY_ASSERT_NOT_NULL(ptr)
#define CHECK(expr) assert((expr))
#define CHECK_EQ(a, b) assert((a) == (b))
#define CHECK_NOT_NULL(ptr) DOBBY_ASSERT_NOT_NULL(ptr)
#define USE(x) (void)(x)

#ifndef UNREACHABLE
#define UNREACHABLE() assert(false && "unreachable")
#endif

#ifdef __cplusplus
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete
#else
#define DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif
