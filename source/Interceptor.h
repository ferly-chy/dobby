#pragma once

#include "dobby/dobby_internal.h"
#include "InterceptEntry.h"

#include <vector>
#include <optional>

class Interceptor {
public:
  static Interceptor *SharedInstance();

public:
  [[nodiscard]] std::optional<InterceptEntry *> find(addr_t addr);

  void remove(addr_t addr);

  void add(InterceptEntry *entry);

  [[nodiscard]] const InterceptEntry *getEntry(int i) const;

  [[nodiscard]] int count() const;

private:
  static Interceptor *instance;

  std::vector<InterceptEntry *> entries;
};