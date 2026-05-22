#include "Interceptor.h"
#include "InterceptRouting/InterceptRouting.h"

#include <algorithm>

Interceptor *Interceptor::SharedInstance() {
  static Interceptor instance;
  return &instance;
}

std::optional<InterceptEntry *> Interceptor::find(addr_t addr) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = std::find_if(entries.begin(), entries.end(), [addr](const auto &entry) {
    return entry->patched_addr == addr;
  });
  
  if (it != entries.end()) {
    return it->get();
  }
  return std::nullopt;
}

void Interceptor::add(std::unique_ptr<InterceptEntry> entry) {
  std::lock_guard<std::mutex> lock(mutex);
  if (transaction_active) {
    staged_entries.push_back(std::move(entry));
  } else {
    // If no transaction, apply immediately
    entry->routing->Commit();
    entries.push_back(std::move(entry));
  }
}

void Interceptor::remove(addr_t addr) {
  std::lock_guard<std::mutex> lock(mutex);
  entries.erase(
    std::remove_if(entries.begin(), entries.end(), [addr](const auto &entry) {
      return entry->patched_addr == addr;
    }),
    entries.end()
  );
}

void Interceptor::transactionBegin() {
  std::lock_guard<std::mutex> lock(mutex);
  transaction_active = true;
}

DobbyStatus Interceptor::transactionCommit() {
  std::lock_guard<std::mutex> lock(mutex);
  if (!transaction_active) {
    return kDobbyFailed;
  }

  // FIXME: Implement "stop-the-world" if possible here for true atomicity
  for (auto &entry : staged_entries) {
    entry->routing->Commit();
    entries.push_back(std::move(entry));
  }
  staged_entries.clear();
  transaction_active = false;
  return kDobbySuccess;
}

const InterceptEntry *Interceptor::getEntry(int i) const {
  std::lock_guard<std::mutex> lock(mutex);
  if (i < 0 || i >= static_cast<int>(entries.size())) {
    return nullptr;
  }
  return entries[i].get();
}

int Interceptor::count() const {
  std::lock_guard<std::mutex> lock(mutex);
  return static_cast<int>(entries.size());
}
