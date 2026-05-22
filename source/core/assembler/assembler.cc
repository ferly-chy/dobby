#include "core/assembler/assembler.h"
#include "logging/logging.h"

namespace zz {

const void *ExternalReference::address() {
  return address_;
}

AssemblerBase::AssemblerBase(void *address) {
  realized_addr_ = address;
}

AssemblerBase::~AssemblerBase() = default;

size_t AssemblerBase::ip_offset() const {
  return reinterpret_cast<CodeBufferBase *>(buffer_.get())->GetBufferSize();
}

size_t AssemblerBase::pc_offset() const {
  return reinterpret_cast<CodeBufferBase *>(buffer_.get())->GetBufferSize();
}

CodeBuffer *AssemblerBase::GetCodeBuffer() {
  return buffer_.get();
}

void AssemblerBase::PseudoBind(AssemblerPseudoLabel *label) {
  auto pc_offset = reinterpret_cast<CodeBufferBase *>(buffer_.get())->GetBufferSize();
  label->bind_to(pc_offset);
  if (label->has_confused_instructions()) {
    label->link_confused_instructions(reinterpret_cast<CodeBufferBase *>(buffer_.get()));
  }
}

void AssemblerBase::RelocBind() {
  for (auto &data_label : data_labels_) {
    PseudoBind(data_label.get());
    reinterpret_cast<CodeBufferBase *>(buffer_.get())->EmitBuffer(data_label->data_, data_label->data_size_);
  }
}

void AssemblerBase::AppendRelocLabel(std::unique_ptr<RelocLabel> label) {
  data_labels_.push_back(std::move(label));
}

void AssemblerBase::SetRealizedAddress(void *address) {
  realized_addr_ = address;
}

void *AssemblerBase::GetRealizedAddress() {
  return realized_addr_;
}

void AssemblerBase::FlushICache(addr_t start, int size) {
}

void AssemblerBase::FlushICache(addr_t start, addr_t end) {
}

} // namespace zz
