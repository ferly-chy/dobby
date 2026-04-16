#pragma once

#include "dobby/common.h"
#include "dobby/platform_mutex.h"
#include "MemoryAllocator/MemoryAllocator.h"
struct InterceptRouting;
struct Interceptor {
  mutable DobbyMutex mutex;

  struct Entry {
    uint32_t id = 0;

    struct {
      bool arm_thumb_mode;
    } features;

    addr_t fake_func_addr;
    dobby_instrument_callback_t pre_handler;
    dobby_instrument_callback_t post_handler;

    addr_t epilogue_dispatch_bridge = 0;

    addr_t addr;

    MemBlock patched;
    MemBlock relocated;

    InterceptRouting *routing = nullptr;

    uint8_t *origin_code_ = nullptr;

    Entry(addr_t addr);

    ~Entry();

    void backup_orig_code();

    void restore_orig_code();

    void feature_set_arm_thumb(bool thumb);
  };

  // Use hash map for O(1) lookup instead of O(n) linear search
  stl::unordered_map<addr_t, Entry *> entries_map;

  static Interceptor *Shared();

  // O(1) lookup by address
  Entry *find(addr_t addr) {
    DobbyLockGuard lock(mutex);
    auto it = entries_map.find(addr);
    if (it != entries_map.end()) {
      return it->second;
    }
    return nullptr;
  }

  // O(1) removal by address
  Entry *remove(addr_t addr) {
    DobbyLockGuard lock(mutex);
    auto it = entries_map.find(addr);
    if (it != entries_map.end()) {
      Entry *entry = it->second;
      entries_map.erase(it);
      return entry;
    }
    return nullptr;
  }

  // O(1) insertion
  void add(Entry *entry) {
    DobbyLockGuard lock(mutex);
    entries_map.insert(stl::pair<addr_t, Entry *>(entry->addr, entry));
  }

  int count() const {
    DobbyLockGuard lock(mutex);
    return entries_map.size();
  }
};

inline static Interceptor gInterceptor;

inline Interceptor *Interceptor::Shared() {
  return &gInterceptor;
}