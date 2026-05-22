#pragma once

#include <vector>
#include <memory>

#include "MemoryAllocator/CodeBuffer/CodeBufferBase.h"

#if defined(TARGET_ARCH_ARM)
#include "MemoryAllocator/CodeBuffer/code_buffer_arm.h"
#elif defined(TARGET_ARCH_ARM64)
#include "MemoryAllocator/CodeBuffer/code_buffer_arm64.h"
#elif defined(TARGET_ARCH_IA32)
#include "MemoryAllocator/CodeBuffer/code_buffer_x86.h"
#elif defined(TARGET_ARCH_X64)
#include "MemoryAllocator/CodeBuffer/code_buffer_x64.h"
#endif

#include "AssemblerPseudoLabel.h"

namespace zz {

class ExternalReference {
public:
  explicit ExternalReference(void *address) : address_(address) {
  }

  const void *address();

private:
  const void *address_;
};

class AssemblerBase {
public:
  explicit AssemblerBase(void *address);

  ~AssemblerBase();

  size_t ip_offset() const;

  size_t pc_offset() const;

  CodeBuffer *GetCodeBuffer();

  void PseudoBind(AssemblerPseudoLabel *label);

  void RelocBind();

  void AppendRelocLabel(std::unique_ptr<RelocLabel> label);

protected:
  std::vector<std::unique_ptr<RelocLabel>> data_labels_;

public:
  virtual void *GetRealizedAddress();

  virtual void SetRealizedAddress(void *address);

  static void FlushICache(addr_t start, int size);

  static void FlushICache(addr_t start, addr_t end);

protected:
  std::shared_ptr<CodeBuffer> buffer_;

  void *realized_addr_;
};

} // namespace zz

#if 0
#include "globals.h"
#if TARGET_ARCH_ARM
#include "core/assembler/assembler-arm.h"
#elif TARGET_ARCH_ARM64
#include "core/assembler/assembler-arm64.h"
#elif TARGET_ARCH_X64
#include "core/assembler/assembler-x64.h"
#else
#error "unsupported architecture"
#endif
#endif