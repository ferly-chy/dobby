#pragma once

#include "dobby.h"
#include <string>
#include <utility>
#include <memory>
#include <stdexcept>

namespace dobby {

class Status {
public:
  Status() : code_(kDobbySuccess) {}
  Status(DobbyError code) : code_(code) {}
  Status(DobbyError code, std::string message) : code_(code), message_(std::move(message)) {}

  static Status Success() { return Status(kDobbySuccess); }
  static Status Error(DobbyError code, std::string message = "") { 
    if (code == kDobbySuccess) return Status(kDobbySuccess);
    return Status(code, std::move(message)); 
  }

  bool ok() const { return code_ == kDobbySuccess; }
  DobbyError code() const { return code_; }
  const std::string& message() const { return message_; }

  explicit operator bool() const { return ok(); }

private:
  DobbyError code_;
  std::string message_;
};

template <typename T>
class Result {
public:
  // Constructor for error status
  Result(Status status) : status_(std::move(status)), value_(nullptr) {
    if (status_.ok()) {
      // Logic error: Result with Success status must have a value.
      // In a real production system we might throw or abort.
      status_ = Status::Error(kDobbyErrorMemoryOperation, "Result created with Success but no value");
    }
  }

  // Constructor for success value
  Result(T value) : status_(Status::Success()), value_(std::make_unique<T>(std::move(value))) {}

  bool ok() const { return status_.ok() && value_ != nullptr; }
  const Status& status() const { return status_; }
  
  T& value() { 
    if (!value_) throw std::runtime_error("Accessing empty Result value: " + status_.message());
    return *value_; 
  }
  
  const T& value() const { 
    if (!value_) throw std::runtime_error("Accessing empty Result value: " + status_.message());
    return *value_; 
  }

  T& operator*() { return value(); }
  T* operator->() { return value_.get(); }

private:
  Status status_;
  std::unique_ptr<T> value_;
};

} // namespace dobby
