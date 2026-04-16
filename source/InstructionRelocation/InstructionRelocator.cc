#include "InstructionRelocation/InstructionRelocator.h"
#include "platform_detect_macro.h"

namespace dobby {

InstructionRelocator::RelocationResult InstructionRelocator::Relocate(addr_t src, 
                                                                    std::span<const uint8_t> code,
                                                                    addr_t dst,
                                                                    bool branch) {
#if defined(TARGET_ARCH_ARM64)
  return RelocateARM64(src, code, dst, branch);
#elif defined(TARGET_ARCH_ARM)
  return RelocateARM(src, code, dst, branch);
#elif defined(TARGET_ARCH_X64)
  return RelocateX64(src, code, dst, branch);
#elif defined(TARGET_ARCH_IA32)
  return RelocateX86(src, code, dst, branch);
#else
  return {};
#endif
}

} // namespace dobby
