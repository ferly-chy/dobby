#include "InstructionRelocation/InstructionRelocation.h"

#include "UniconEmulator.h"

static void check_insn_relo_x64_case(const char *buffer, size_t buffer_size, bool check_fault_addr, int check_reg_id,
                                     void (^callback)(UniconEmulator *orig, UniconEmulator *relo)) {
  alignas(4) char code[64] = {0};
  memcpy(code, buffer, buffer_size);
  check_insn_relo(code, buffer_size, check_fault_addr, check_reg_id, callback);
}

int main() {
  set_global_arch("x86_64");

  // cmp eax, eax
  // jz -0x20
  check_insn_relo_x64_case("\x39\xc0\x74\xdc", 4, false, UC_X86_REG_IP, nullptr);
  // cmp eax, eax
  // jz 0x20
  check_insn_relo_x64_case("\x39\xc0\x74\x1c", 4, false, UC_X86_REG_IP, nullptr);

  // jmp -0x20
  check_insn_relo_x64_case("\xeb\xde", 2, false, UC_X86_REG_IP, nullptr);
  // jmp 0x20
  check_insn_relo_x64_case("\xeb\x1e", 2, false, UC_X86_REG_IP, nullptr);

  // jmp -0x4000
  check_insn_relo_x64_case("\xe9\xfb\xbf\xff\xff", 5, false, UC_X86_REG_IP, nullptr);
  // jmp 0x4000
  check_insn_relo_x64_case("\xe9\xfb\x3f\x00\x00", 5, false, UC_X86_REG_IP, nullptr);

  // lea rax, [rip]
  check_insn_relo_x64_case("\x48\x8d\x05\x00\x00\x00\x00", 7, false, UC_X86_REG_RAX, nullptr);

  // lea rax, [rip + 0x4000]
  check_insn_relo_x64_case("\x48\x8d\x05\x00\x40\x00\x00", 7, false, UC_X86_REG_RAX, nullptr);

  // mov rax, [rip + 0x4000]
  check_insn_relo_x64_case("\x48\x8b\x05\x00\x40\x00\x00", 7, true, -1, nullptr);

  return 0;
}
