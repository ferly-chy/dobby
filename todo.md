# Dobby Modernization Roadmap

This document outlines the proposed upgrades for plugins and external libraries to make the Dobby fork more modern, maintainable, and powerful.

## 1. Core Library Upgrades & Analysis

### A. Logging System
*   **Current State**: Partially integrated `spdlog`. Thin wrapper in `include/logging/logging.h`.
*   **Recommendation**: **Deep spdlog Integration**
    *   **Action**: Implement custom sinks for Android Logcat and iOS `os_log`.
    *   **Reasoning**: `spdlog` is incredibly fast (asynchronous mode) and allows structured logging which is vital for debugging complex hooks.
    *   **Impact**: Better diagnostic capabilities without compromising performance.
*   **Priority**: **High**

### B. Standard Library (STL)
*   **Current State**: Mid-migration from `TINYSTL` to `std::`. `TINYSTL` still exists in several core headers.
*   **Recommendation**: **Full migration to libc++ (Standard STL)**
    *   **Action**: Remove all `tinystl` namespaces and replace with `std`. Use `std::span` (C++20) and `std::string_view` for memory/string operations.
    *   **Reasoning**: Modern Android NDKs provide high-quality `libc++`. Reinventing the STL adds maintenance burden and prevents using modern C++ features like `<algorithm>` and `<ranges>`.
    *   **Impact**: Significantly improved maintainability and reliability.
*   **Priority**: **Critical**

### C. Async & Network
*   **Recommended**: **[Asio](https://think-async.com/Asio/)** (Standalone version)
    *   **Use Case**: Asynchronous event reporting, remote monitoring, or network-based hook control.
    *   **Reasoning**: Asio is the foundation of the networking TS and provides a very clean, performant async runtime without the weight of Boost.
    *   **Priority**: **Low/Medium** (Depending on feature requirements).

### D. Configuration
*   **Current State**: Hardcoded logic or simple `nlohmann/json`.
*   **Recommended**: **[toml++](https://github.com/marzer/tomlplusplus)**
    *   **Reasoning**: TOML is more human-readable and editable than JSON, making it ideal for configuration files used by researchers and developers.
    *   **Priority**: **Low**.

---

## 2. Architectural Improvements

### A. Modular Plugin System
*   **Current State**: Static registration or basic interface-based registry.
*   **Recommendation**: **Event-Driven Plugin Architecture**
    *   **Design**: Implement a `PluginHost` that uses `std::expected` for error propagation and supports dynamic loading of `.so` / `.dylib` files.
    *   **Key Idea**: Plugins should be able to subscribe to events like `onHookCreated`, `onInstructionRelocated`, or `onModuleLoaded`.
*   **Priority**: **Medium**.

### B. Error Handling
*   **Current State**: Often returns `void` or simple status codes.
*   **Recommendation**: **`std::expected` (C++23) or `outcome`**
    *   **Impact**: Clean, modern error handling that forces developers to consider failure states without the overhead of exceptions (crucial for hook libraries).

---

## 3. Specific Plugin Modernization

### A. SymbolResolver Refactoring
*   **Current State**: Manual, low-level ELF/Mach-O parsing.
*   **Recommendation**: Refactor using a more robust internal abstraction.
    *   **Goal**: Better support for `DT_GNU_HASH`, compressed sections, and symbol versioning.
    *   **Option**: Consider a lightweight, header-only library like **[ELFIO](https://github.com/serge1/ELFIO)** if `LIEF` is too heavy.

### B. ApplicationEventMonitor
*   **Action**: Decouple specific monitors (File, Socket, Memory) into individual plugins.
*   **Benefit**: Users can enable only what they need, reducing the footprint and potential side effects.

---

## 4. Migration Plan (Step-by-Step)

### Phase 1: Foundation (The "Standardization" Phase)
1.  **Complete STL Migration**: [x] Search and replace all `tinystl` usages. Update `platform_features.h`.
2.  **Modernize CMake**: [x] Ensure `FetchContent` handles all dependencies cleanly (Added toml++, ELFIO).
3.  **Deepen Logging**: [x] Replace C-style `printf` in plugins with `spdlog` macros.

### Phase 2: Refactoring (The "Decoupling" Phase)
1.  **Isolate Plugins**: [ ] Move code from `builtin-plugin` into a more structured directory with clear interfaces. (Started: `source/Plugins` created, `IPlugin` interface defined).
2.  **Refactor SymbolResolver**: [x] Create a clean `ElfParser` class to replace global C structs. (Implemented `ElfSymbolResolver` class).
3.  **Implement Error Handling**: [ ] Transition internal APIs to return `std::expected`. (Started using `std::expected` in `IPlugin`).

### Phase 3: Extension (The "Power" Phase)
1.  **Dynamic Loading**: Add support for loading external plugins at runtime.
2.  **Config Loader**: Implement the TOML config system for fine-tuned control of hooks.
3.  **Async Integration**: (Optional) Add Asio for background telemetry.

---

## 5. Comparison Table

| Feature | Current | Modern Alternative | Advantage |
| :--- | :--- | :--- | :--- |
| **Logging** | Custom/Thin spdlog | Full spdlog | Structured, Async, Sinks (Logcat) |
| **Containers** | TINYSTL | libc++ (std) | Standard compliance, safety, perf |
| **Parsing** | Manual C-style | C++ Classes / ELFIO | Robustness, readability |
| **Config** | JSON/Hardcoded | TOML (toml++) | Human-readability |
| **Errors** | Status Codes | std::expected | Type-safety, explicit handling |
| **Plugins** | Static Registry | Event-Driven Host | Modularity, Extensibility |

---

## 6. Possible Issues & Breaking Changes
*   **Binary Size**: `libc++` and `spdlog` will increase the base footprint. Mitigate with `-Os` and LTO.
*   **C++ Standard**: Requires a modern compiler (Clang/GCC with C++26 support).
*   **Symbol Visibility**: Ensure plugins can correctly resolve symbols from the main Dobby library when loaded dynamically.
