#pragma once

#include <vector>
#include <span>
#include <expected>

#include "dobby/common.h"

class CodeBufferBase {
public:
  CodeBufferBase() = default;

public:
  [[nodiscard]] virtual std::unique_ptr<CodeBufferBase> Copy();

  void Emit8(uint8_t data);

  void Emit16(uint16_t data);

  void Emit32(uint32_t data);

  void Emit64(uint64_t data);

  template <typename T>
  [[nodiscard]] std::expected<T, int> Load(size_t offset) const {
    if (offset + sizeof(T) > buffer_.size()) {
      return std::unexpected(-1);
    }
    return *(const T *)(buffer_.data() + offset);
  }

  template <typename T>
  bool Store(size_t offset, T value) {
    if (offset + sizeof(T) > buffer_.size()) {
      return false;
    }
    *(T *)(buffer_.data() + offset) = value;
    return true;
  }

  template <typename T>
  void Emit(T value) {
    EmitBuffer((const uint8_t *)&value, sizeof(value));
  }

  void EmitBuffer(std::span<const uint8_t> buffer);

  void EmitBuffer(const uint8_t *buffer, size_t len) {
    EmitBuffer(std::span<const uint8_t>(buffer, len));
  }

  [[nodiscard]] std::span<uint8_t> GetBuffer();
  [[nodiscard]] size_t GetBufferSize() const;

private:
  std::vector<uint8_t> buffer_;
};
