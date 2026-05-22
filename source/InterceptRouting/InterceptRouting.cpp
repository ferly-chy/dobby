#include "dobby/dobby_internal.h"

#include "InterceptRouting/InterceptRouting.h"
#include "InterceptRouting/RoutingPlugin/RoutingPlugin.h"

#include <fmt/format.h>

using namespace zz;

void log_hex_format(uint8_t *buffer, uint32_t buffer_size) {
  std::string output;
  for (uint32_t i = 0; i < buffer_size; i++) {
    output += fmt::format("{:02x}", buffer[i]);
  }
  DEBUG_LOG("{}", output);
};

void InterceptRouting::Prepare() {
}

std::expected<void, int> InterceptRouting::GenerateRelocatedCode() {
  uint32_t tramp_size = GetTrampolineBuffer()->GetBufferSize();
  origin_ = std::make_unique<CodeMemBlock>(entry_->patched_addr, tramp_size);
  relocated_ = std::make_unique<CodeMemBlock>();

  auto buffer = (void *)entry_->patched_addr;
#if defined(TARGET_ARCH_ARM)
  if (entry_->thumb_mode) {
    buffer = (void *)((addr_t)buffer + 1);
  }
#endif
  GenRelocateCodeAndBranch(buffer, origin_.get(), relocated_.get());
  if (relocated_->size == 0) {
    ERROR_LOG("[insn relocate] failed");
    return std::unexpected(-1);
  }

  // set the relocated instruction address
  entry_->relocated_addr = relocated_->addr;

  // save original prologue
  memcpy((void *)entry_->origin_insns, (void *)origin_->addr, origin_->size);
  entry_->origin_insn_size = origin_->size;

  // log
  DEBUG_LOG("[insn relocate] origin {:p} - {}", (void *)origin_->addr, origin_->size);
  log_hex_format((uint8_t *)origin_->addr, origin_->size);

  DEBUG_LOG("[insn relocate] relocated {:p} - {}", (void *)relocated_->addr, relocated_->size);
  log_hex_format((uint8_t *)relocated_->addr, relocated_->size);

  return {};
}

std::expected<void, int> InterceptRouting::GenerateTrampolineBuffer(addr_t src, addr_t dst) {
  // if near branch trampoline plugin enabled
  if (RoutingPluginManager::near_branch_trampoline) {
    auto plugin = static_cast<RoutingPluginInterface *>(RoutingPluginManager::near_branch_trampoline);
    if (plugin->GenerateTrampolineBuffer(this, src, dst) == false) {
      DEBUG_LOG("Failed enable near branch trampoline plugin");
    }
  }

  if (GetTrampolineBuffer() == nullptr) {
    auto tramp_buffer = GenerateNormalTrampolineBuffer(src, dst);
    SetTrampolineBuffer(std::move(tramp_buffer));
  }
  return {};
}

// active routing, patch origin instructions as trampoline
void InterceptRouting::Active() {
  auto buffer_span = trampoline_buffer_->GetBuffer();
  auto ret = DobbyCodePatch((void *)entry_->patched_addr, buffer_span.data(),
                            (uint32_t)buffer_span.size());
  if (ret == -1) {
    ERROR_LOG("[intercept routing] active failed");
    return;
  }
  DEBUG_LOG("[intercept routing] active");
}

void InterceptRouting::Commit() {
  this->Active();
}

#if 0
int InterceptRouting::PredefinedTrampolineSize() {
#if __arm64__
  return 12;
#elif __arm__
  return 8;
#endif
}
#endif

InterceptEntry *InterceptRouting::GetInterceptEntry() {
  return entry_;
};
