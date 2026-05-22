# Dobby Modernization Roadmap (2026-05-21)

This document tracks the evolution of Dobby into a modern, high-performance, and extensible hooking framework.

## 🏁 Completed Checkpoints (May 2026)
*   **Modern Baseline**: Migrated to C++26 and C23.
*   **Vendor Slimming**: Trimmed `fmt`, `spdlog`, `json`, `toml++`, and `elfio` to remove bloat.
*   **Plugin Architecture**: Implemented `PluginManager` and migrated SymbolResolver to a robust **ELFIO-based** engine.
*   **Typed Closure Metadata**: Replaced `void*` carry data with a tagged union model.
*   **Cross-Arch Validation**: Verified builds for `arm64-v8a`, `armeabi-v7a`, `x86`, and `x86_64`.

---

## 🛠️ Immediate Technical Debt & Missing Features (Phase 2.5)

### 1. RAII & Memory Safety (High Priority)
The core APIs currently use manual `new` and raw pointers, which are prone to leaks during failure states.
*   **DONE**: Refactor `Interceptor` to use `std::vector<std::unique_ptr<InterceptEntry>>` and thread-safe singleton.
*   **DONE**: Use `std::unique_ptr` for `InterceptEntry` and `Routing` objects within `DobbyHook` and `DobbyInstrument`.
*   **DONE**: Replace raw pointer management in `source/core/` (Assembler/CodeBuffer) with RAII patterns.

### 2. Missing Core Implementation: PLT/GOT (High Priority)
`DobbyImportTableReplace` is declared in `dobby.h` but lacks implementation in `source/`.
*   **DONE**: Implement `DobbyImportTableReplace` using the `ELFIO` library. (Verified implementation in `ImportTableResolver.cc`)
*   **Goal**: Provide a clean way to hook imported functions by patching the Global Offset Table.

### 3. Robust Hook Destruction (Medium Priority)
`DobbyDestroy` currently only patches the code back without considering thread safety or comprehensive resource cleanup.
*   **Task**: Implement thread-safe destruction (ensure no threads are executing the trampoline during removal).
*   **DONE**: Ensure `relocated_addr` and associated memory blocks (via `InterceptRouting`) are properly released.

### 4. Logging & Internal API Standardization (Medium Priority)
*   **DONE**: Replace legacy `snprintf` hex logging in `InterceptRouting.cpp` with `spdlog`/`fmt`.
*   **DONE**: Refactor `source/core/` (assembler/codegen) to use `std::span` for buffer management in `CodeBufferBase`.
*   **DONE**: Create a unified error mapping between internal `PluginError` and public `DobbyStatus`. (Added ToDobbyStatus helper logic)

---

## 🚀 Future Suggestions & Strategic Roadmap

### 1. Hook Transactions (Atomic Batching)
**Priority: High** | **Difficulty: Medium**
Allow multiple hooks to be prepared and applied in a single "stop-the-world" transaction.
*   **Safety**: Keep existing `DobbyHook` as a thin wrapper around a single-item transaction.

### 2. Dynamic Plugin Host (.so / .dylib Loading)
**Priority: Medium** | **Difficulty: Medium**
Enable Dobby to load external plugins at runtime using `dlopen`.

### 3. Event-Driven Plugin Bus (Pub/Sub)
**Priority: Medium** | **Difficulty: Low**
Allow plugins to subscribe to internal events like `ON_HOOK_CREATED`.

### 4. Modern Memory Management (std::pmr)
**Priority: Low** | **Difficulty: High**
Refactor the custom `MemoryAllocator` to use C++17/20 Polymorphic Memory Resources.

---

## 🧪 VPS / Cross-ABI Follow-up (Unfinished)

### Completed in current Termux session
*   **DONE**: Host-side tests now pass on Termux via `scripts/test_runner.sh`:
    *   `test_native`
    *   `test_vtable`
    *   `test_insn_relo_arm64`
*   **DONE**: Added CTest integration for current host-runnable tests.
*   **DONE**: Modernized shared relocation test harness (`UniconEmulator.*`) and reduced noisy output.
*   **DONE**: Fixed stale `test_native.cpp` and modernized `test_insn_decoder_x86.cpp` to reuse the shared harness.
*   **DONE**: Fixed Android example link issue in `examples/CMakeLists.txt` by removing invalid `logging` link dependency from `socket_example`.
*   **DONE**: Made Android test target selection follow `ANDROID_ABI` instead of the Termux host CPU.

### Remaining blocker for full Android all-ABI test builds
Android cross-builds for x86 / x86_64 still fail in test targets because Unicorn/Capstone are being resolved from Termux host libraries under `/data/data/com.termux/files/usr/lib`, which are `aarch64` host binaries and incompatible with x86/x86_64 Android targets.

#### Current failure mode
*   `test_insn_relo_x64` and `test_insn_decoder_x86` attempt to link:
    *   `/data/data/com.termux/files/usr/lib/libcapstone.so`
    *   `/data/data/com.termux/files/usr/lib/libunicorn.so`
*   These are host-side Termux libraries, not Android target-ABI libraries.
*   Result: Android x86/x86_64 validation still fails even though core `dobby`, `dobby_static`, and examples are much closer to green.

### Recommended next step on VPS
#### Option A — Recommended (pragmatic)
Make Unicorn/Capstone relocation tests **host-only**, and skip those test targets entirely for Android cross-build validation.

Suggested implementation direction:
1. In `tests/CMakeLists.txt`, only configure Unicorn/Capstone-backed relocation test executables for:
   *   native host builds, or
   *   Android builds where ABI-matched Unicorn/Capstone are explicitly available.
2. Keep Android all-ABI validation focused on:
   *   `dobby`
   *   `dobby_static`
   *   examples that do not depend on host-only libraries
3. Continue using `scripts/test_runner.sh` / `ctest` for host runtime validation.

#### Option B — Full cross-ABI test support
Provide Android ABI-matched Unicorn + Capstone for every target ABI (`arm64-v8a`, `armeabi-v7a`, `x86`, `x86_64`) and update test discovery/linking to use those instead of Termux host libs.

This is more complete, but significantly larger in scope.

### Suggested VPS checklist
1. Reproduce current Android all-ABI build:
   *   `for abi in arm64-v8a armeabi-v7a x86 x86_64; do ANDROID_ABI="$abi" bash ./scripts/build_and_test_all.sh; done`
2. Patch `tests/CMakeLists.txt` so Android cross-builds skip Unicorn/Capstone relocation tests unless ABI-matched deps exist.
3. Re-run all ABI builds.
4. If desired, add a separate CMake option such as:
   *   `DOBBY_BUILD_HOST_ONLY_RELOCATION_TESTS`
   *   or `DOBBY_ENABLE_UNICORN_TESTS`
5. If VPS environment has proper Android prebuilt deps, optionally implement Option B.

---

## 🛠️ Operational Instructions for Next Steps

1.  **For RAII Migration**:
    *   Update `source/Interceptor.h` to use `std::unique_ptr`.
    *   Audit `source/InterceptRouting/Routing/` for raw `new` calls.
2.  **For PLT/GOT**:
    *   Create `source/Plugins/SymbolResolver/ImportTableResolver.cc`.
    *   Use `ELFIO` to locate `.got` and `.plt.got` sections.
3.  **For Transactions**:
    *   Create `source/HookTransaction.h`.
    *   Move core patching logic from `Interceptor` to the Transaction class.

---
*Note: Always run `./scripts/build_and_test_all.sh` after any change to core logic.*
