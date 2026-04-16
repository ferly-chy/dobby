#include "InstructionRelocation/InstructionRelocator.h"
#include "platform_detect_macro.h"

#if defined(TARGET_ARCH_ARM64)

#include "InstructionRelocation/arm64/InstructionRelocationARM64.h"
#include "dobby/dobby_internal.h"
#include "PlatformUnifiedInterface/platform.h"
#include "core/arch/arm64/registers-arm64.h"
#include "core/assembler/assembler-arm64.h"
#include "core/codegen/codegen-arm64.h"
#include "inst_constants.h"
#include "inst_decode_encode_kit.h"

using namespace zz::arm64;

namespace dobby {

struct relo_ctx_t {
  addr_t cursor;
  std::span<const uint8_t> input_code;
  TurboAssembler *turbo_assembler;

  relo_ctx_t(addr_t src, std::span<const uint8_t> code, TurboAssembler *ta) 
    : cursor(src), input_code(code), turbo_assembler(ta) {}

  uint32_t relo_size() const { return cursor - (addr_t)input_code.data(); } // This is wrong, should track relative to start
};

// Fixed context tracker
struct ReloContext {
  addr_t src_base;
  std::span<const uint8_t> input;
  uint32_t offset;
  TurboAssembler *as;

  ReloContext(addr_t src, std::span<const uint8_t> code, TurboAssembler *ta) 
    : src_base(src), input(code), offset(0), as(ta) {}

  addr_t cur_src_addr() const { return src_base + offset; }
  const uint8_t* cur_input_ptr() const { return input.data() + offset; }
  bool has_more() const { return offset < input.size(); }
};

#undef _
#define _ ctx.as->

static inline bool inst_is_b_bl(uint32_t instr) { return (instr & UnconditionalBranchFixedMask) == UnconditionalBranchFixed; }
static inline bool inst_is_ldr_literal(uint32_t instr) { return ((instr & LoadRegLiteralFixedMask) == LoadRegLiteralFixed); }
static inline bool inst_is_adr(uint32_t instr) { return (instr & PCRelAddressingFixedMask) == PCRelAddressingFixed && (instr & PCRelAddressingMask) == ADR; }
static inline bool inst_is_adrp(uint32_t instr) { return (instr & PCRelAddressingFixedMask) == PCRelAddressingFixed && (instr & PCRelAddressingMask) == ADRP; }
static inline bool inst_is_b_cond(uint32_t instr) { return (instr & ConditionalBranchFixedMask) == ConditionalBranchFixed; }
static inline bool inst_is_compare_b(uint32_t instr) { return (instr & CompareBranchFixedMask) == CompareBranchFixed; }
static inline bool inst_is_test_b(uint32_t instr) { return (instr & TestBranchFixedMask) == TestBranchFixed; }

InstructionRelocator::RelocationResult InstructionRelocator::RelocateARM64(addr_t src, std::span<const uint8_t> code, addr_t dst, bool branch) {
  TurboAssembler turbo_assembler(dst);
  ReloContext ctx(src, code, &turbo_assembler);

  while (ctx.has_more()) {
    uint32_t inst = *(uint32_t *)ctx.cur_input_ptr();
    addr_t cur_addr = ctx.cur_src_addr();
    
    if (inst_is_b_bl(inst)) {
      int64_t offset = decode_imm26_offset(inst);
      addr_t target = cur_addr + offset;
      auto data_label = turbo_assembler.createDataLabel(target);
      _ Ldr(TMP_REG_0, data_label);
      if ((inst & UnconditionalBranchMask) == BL) {
        _ blr(TMP_REG_0);
      } else {
        _ br(TMP_REG_0);
      }
    } else if (inst_is_ldr_literal(inst)) {
      int64_t offset = decode_imm19_offset(inst);
      addr_t target = cur_addr + offset;
      int rt = decode_rt(inst);
      char opc = bits(inst, 30, 31);
      _ Mov(TMP_REG_0, target);
      if (opc == 0b00) _ ldr(W(rt), MemOperand(TMP_REG_0, 0));
      else if (opc == 0b01) _ ldr(X(rt), MemOperand(TMP_REG_0, 0));
    } else if (inst_is_adr(inst)) {
      int64_t offset = decode_immhi_immlo_offset(inst);
      addr_t target = cur_addr + offset;
      _ Mov(X(decode_rd(inst)), target);
    } else if (inst_is_adrp(inst)) {
      int64_t offset = decode_immhi_immlo_zero12_offset(inst);
      uintptr_t page_mask = ~(uintptr_t)(OSMemory::PageSize() - 1);
      addr_t target = (cur_addr & page_mask) + offset;
      _ Mov(X(decode_rd(inst)), target);
    } else if (inst_is_b_cond(inst) || inst_is_compare_b(inst) || inst_is_test_b(inst)) {
      int64_t offset = 0;
      uint32_t branch_inst = inst;
      if (inst_is_b_cond(inst)) {
        offset = decode_imm19_offset(inst);
        char cond = (char)(bits(inst, 0, 3) ^ 1);
        set_bits(branch_inst, 0, 3, cond);
        set_bits(branch_inst, 5, 23, (4 * 3) >> 2);
      } else if (inst_is_compare_b(inst)) {
        offset = decode_imm19_offset(inst);
        char op = (char)(bit(inst, 24) ^ 1);
        set_bit(branch_inst, 24, op);
        set_bits(branch_inst, 5, 23, (4 * 3) >> 2);
      } else if (inst_is_test_b(inst)) {
        offset = decode_imm14_offset(inst);
        char op = (char)(bit(inst, 24) ^ 1);
        set_bit(branch_inst, 24, op);
        set_bits(branch_inst, 5, 18, (4 * 3) >> 2);
      }
      addr_t target = cur_addr + offset;
      auto data_label = turbo_assembler.createDataLabel(target);
      _ Emit(branch_inst);
      _ Ldr(TMP_REG_0, data_label);
      _ br(TMP_REG_0);
    } else {
      _ Emit(inst);
    }
    ctx.offset += 4;
  }

  if (branch) {
    CodeGen codegen(&turbo_assembler);
    codegen.LiteralLdrBranch(ctx.cur_src_addr());
  }

  turbo_assembler.relocDataLabels();
  
  // Extract bytes WITHOUT permanent execution memory allocation
  auto code_buffer = turbo_assembler.code_buffer();
  std::vector<uint8_t> relocated_code(code_buffer->data(), code_buffer->data() + code_buffer->size());
  
  return {relocated_code, ctx.offset, dst};
}

} // namespace dobby

void GenRelocateCode(void *buffer, CodeMemBlock *origin, CodeMemBlock *relocated, bool branch) {
  auto result = dobby::InstructionRelocator::Relocate((addr_t)origin->addr(), 
                                                     std::span((const uint8_t*)origin->addr(), origin->size),
                                                     (addr_t)buffer, branch);
  
  // Legacy API expects code to be WRITTEN to buffer
  DobbyCodePatch(buffer, result.code.data(), result.code.size());
  
  relocated->start_ = (addr_t)buffer;
  relocated->size = result.code.size();
}

void GenRelocateCodeAndBranch(void *buffer, CodeMemBlock *origin, CodeMemBlock *relocated) {
  GenRelocateCode(buffer, origin, relocated, true);
}

#endif
