#include "Interceptor.h"

Interceptor *Interceptor::instance = nullptr;

Interceptor *Interceptor::SharedInstance() {
  if (Interceptor::instance == nullptr) {
    Interceptor::instance = new Interceptor();
  }
  return Interceptor::instance;
}

std::optional<InterceptEntry *> Interceptor::find(addr_t addr) {
  for (auto *entry : entries) {
    if (entry->patched_addr == addr) {
      return entry;
    }
  }
  return std::nullopt;
}

void Interceptor::add(InterceptEntry *entry) {
  entries.push_back(entry);
}

void Interceptor::remove(addr_t addr) {
  for (auto iter = entries.begin(); iter != entries.end(); iter++) {
    if ((*iter)->patched_addr == addr) {
      entries.erase(iter);
      break;
    }
  }
}

const InterceptEntry *Interceptor::getEntry(int i) const {
  return entries[i];
}

int Interceptor::count() const {
  return static_cast<int>(entries.size());
}
