#pragma once

#include "InterceptEntry.h"
#include "MemoryAllocator/AssemblyCodeBuilder.h"
#include "InstructionRelocation/InstructionRelocation.h"
#include "TrampolineBridge/Trampoline/Trampoline.h"

#include <expected>

class InterceptRouting {
public:
  explicit InterceptRouting(InterceptEntry *entry) : entry_(entry) {
    trampoline_target_ = 0;
  }

  virtual ~InterceptRouting() = default;

  virtual void DispatchRouting() = 0;

  virtual void Prepare();

  virtual void Active();

  void Commit();

  InterceptEntry *GetInterceptEntry();

  void SetTrampolineBuffer(std::unique_ptr<CodeBufferBase> buffer) {
    trampoline_buffer_ = std::move(buffer);
  }

  CodeBufferBase *GetTrampolineBuffer() {
    return trampoline_buffer_.get();
  }

  void SetTrampolineTarget(addr_t address) {
    trampoline_target_ = address;
  }

  addr_t GetTrampolineTarget() {
    return trampoline_target_;
  }

protected:
  std::expected<void, int> GenerateRelocatedCode();

  std::expected<void, int> GenerateTrampolineBuffer(addr_t src, addr_t dst);

protected:
  InterceptEntry *entry_;

  std::unique_ptr<CodeMemBlock> origin_;
  std::unique_ptr<CodeMemBlock> relocated_;

  std::unique_ptr<CodeMemBlock> trampoline_;
  // trampoline buffer before active
  std::unique_ptr<CodeBufferBase> trampoline_buffer_;
  addr_t trampoline_target_;
};
