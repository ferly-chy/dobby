#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#if defined(__ANDROID__) || defined(__linux__)
#define OS_ANDROID 1
#define OS_POSIX 1
#include <unistd.h>
#else
#error "Dobby-Fork Android only supports Android/Linux"
#endif

#if defined(__GNUC__)
#define COMPILER_GCC 1
#else
#error "Dobby-Fork Android requires GCC/Clang"
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86_64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__arm__) || defined(__ARMEL__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARMEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARM64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#else
#error "Unsupported architecture"
#endif

#endif
