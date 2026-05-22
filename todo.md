# Dobby Refactor & Modernization Roadmap (2026-05-22)

## 🏁 Completed Milestones
*   **C++26 Baseline**: Fully migrated to the latest standard.
*   **RAII & Memory Safety**: Interceptor and CodeBuffer now use `std::unique_ptr` and `std::vector`.
*   **Modern Logging**: Integrated `spdlog` and `fmt` throughout the core.
*   **PLT/GOT Engine**: Robust ELFIO-based import table patching implemented.
*   **Android/Termux Support**: Host-side tests passing and ABI selection logic fixed.

---

## 🏗️ Phase 3: Architectural Refactoring (Current Focus)

### 1. Unified Backend Interface
**Goal**: Decouple the `Interceptor` from architecture-specific details.
*   [ ] Refactor `Backend` to use a polymorphic interface instead of heavy `#ifdef` branching in `Interceptor.cpp`.
*   [ ] Standardize `InstructionRelocation` return types and error handling across all architectures.

### 2. Modern Assembler & Codegen
**Goal**: Reduce code duplication and improve type safety in the JIT core.
*   [ ] Replace raw byte-pushing in `Assembler` with a safer `Instruction` object model where possible.
*   [ ] Audit `source/core/arch/` for remaining raw pointer leaks or unsafe memory access.
*   [ ] Refactor `AssemblyCodeBuilder` to use `std::span` and `std::vector<uint8_t>` exclusively.

### 3. Thread-Safe Hook Lifecycle
**Goal**: Eliminate race conditions during hook creation and destruction.
*   [ ] **DobbyDestroy**: Implement a mechanism (e.g., "Stop-the-World" or task-wait) to ensure no threads are executing the trampoline during hook removal.
*   [ ] **Atomic Transactions**: Implement `DobbyTransactionBegin/End` to apply multiple hooks as a single atomic operation.

### 4. Memory Management (PMR)
**Goal**: Replace custom/manual `MemoryAllocator` with standard C++ Polymorphic Memory Resources.
*   [ ] Transition `CodeBuffer` and internal metadata caches to use `std::pmr::monotonic_buffer_resource` for better performance and leak prevention.

---

## 🧪 CI/CD & Portability Fixes

### 1. Android Cross-Build Cleanup
*   [ ] Fix x86/x86_64 test linking by excluding host-side Unicorn/Capstone in `tests/CMakeLists.txt` for cross-compilation.
*   [ ] Standardize the test runner to skip host-specific tests when targeting Android ABIs.

---

## 🚀 Strategic Long-Term Goals
*   **Dynamic Plugin System**: Enable runtime `.so` / `.dylib` loading via `dlopen` for modular extensions.
*   **Event Bus**: Implement an internal Pub/Sub system for hook-related events (e.g., `ON_HOOK_CREATED`, `ON_HOOK_DESTROYED`).
*   **Kernel Mode Support**: Refactor core primitives to ensure portability to Ring 0 environments.

---
*Note: Always verify changes with `./scripts/build_and_test_all.sh`.*
