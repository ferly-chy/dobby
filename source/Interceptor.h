#pragma once

#include "dobby/dobby_internal.h"
#include "InterceptEntry.h"

#include <vector>
#include <optional>
#include <memory>
#include <mutex>

class Interceptor {
public:
  static Interceptor *SharedInstance();

public:
  [[nodiscard]] std::optional<InterceptEntry *> find(addr_t addr);

  void remove(addr_t addr);

  void add(std::unique_ptr<InterceptEntry> entry);

  void transactionBegin();
  DobbyStatus transactionCommit();

  [[nodiscard]] const InterceptEntry *getEntry(int i) const;

  [[nodiscard]] int count() const;

private:
  Interceptor() = default;
  ~Interceptor() = default;

  Interceptor(const Interceptor &) = delete;
  Interceptor &operator=(const Interceptor &) = delete;

  std::vector<std::unique_ptr<InterceptEntry>> entries;
  std::vector<std::unique_ptr<InterceptEntry>> staged_entries;
  bool transaction_active = false;
  mutable std::mutex mutex;
};
