#pragma once

#include "dobby.h"
#include "dobby/types.h"
#include "dobby/platform_features.h"
#include "dobby/platform_detect_macro.h"
#include "dobby/utility_macro.h"
#include "dobby/pac_kit.h"

#include "logging/logging.h"
#include "base/check_logging.h"

#ifdef __cplusplus
#include <fmt/format.h>
#include <tuple>

namespace dobby {
template <typename... Args>
void debug_print_first(Args... args) {
  if constexpr (sizeof...(Args) > 0) {
    // C++26 Pack Indexing
    fmt::print("First arg: {}\n", args...[0]);
  }
}
} // namespace dobby
#endif