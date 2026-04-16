#pragma once

#include <vector>
#include <cstdint>
#include <span>
#include "dobby.h"
#include "dobby/common.h"

namespace dobby {

/**
 * @brief Modern Instruction Relocator
 * 
 * Provides a stateless, memory-safe interface for relocating instructions
 * from one address to another while maintaining PC-relative addressing.
 */
class InstructionRelocator {
public:
  struct RelocationResult {
    std::vector<uint8_t> code;
    uint32_t original_size;
    addr_t target_addr;
  };

  /**
   * @brief Relocate a block of instructions
   * 
   * @param source_addr Original address of the instructions
   * @param source_code Span containing the original instruction bytes
   * @param target_addr Destination address where relocated code will be placed
   * @param add_branch If true, adds a branch back to the original code after the block
   * @return RelocationResult containing the new code and metadata
   */
  static RelocationResult Relocate(addr_t source_addr, 
                                 std::span<const uint8_t> source_code,
                                 addr_t target_addr,
                                 bool add_branch = true);

private:
  // Internal implementation helpers per architecture
#if defined(TARGET_ARCH_ARM64)
  static RelocationResult RelocateARM64(addr_t src, std::span<const uint8_t> code, addr_t dst, bool branch);
#elif defined(TARGET_ARCH_ARM)
  static RelocationResult RelocateARM(addr_t src, std::span<const uint8_t> code, addr_t dst, bool branch);
#elif defined(TARGET_ARCH_X64)
  static RelocationResult RelocateX64(addr_t src, std::span<const uint8_t> code, addr_t dst, bool branch);
#elif defined(TARGET_ARCH_IA32)
  static RelocationResult RelocateX86(addr_t src, std::span<const uint8_t> code, addr_t dst, bool branch);
#endif
};

} // namespace dobby
