#pragma once

#ifndef SPDLOG_NO_EXCEPTIONS
#define SPDLOG_NO_EXCEPTIONS
#endif

#ifndef FMT_USE_EXCEPTIONS
#define FMT_USE_EXCEPTIONS 0
#endif

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#define LOG_LEVEL_DEBUG spdlog::level::debug
#define LOG_LEVEL_INFO  spdlog::level::info
#define LOG_LEVEL_WARN  spdlog::level::warn
#define LOG_LEVEL_ERROR spdlog::level::err
#define LOG_LEVEL_FATAL spdlog::level::critical

#define DEBUG_LOG(...) spdlog::debug(__VA_ARGS__)
#define INFO_LOG(...)  spdlog::info(__VA_ARGS__)
#define WARN_LOG(...)  spdlog::warn(__VA_ARGS__)
#define ERROR_LOG(...) spdlog::error(__VA_ARGS__)
#define FATAL_LOG(...) spdlog::critical(__VA_ARGS__)

#define LOG(level, ...) spdlog::log(level, __VA_ARGS__)

#define UNIMPLEMENTED() FATAL_LOG("unimplemented code at {}:{}", __FILE__, __LINE__)
#define UNREACHABLE()   FATAL_LOG("unreachable code at {}:{}", __FILE__, __LINE__)

inline void logger_set_options(void* logger, const char* tag, const char* file, int level, bool time, bool syslog) {
    spdlog::set_level(static_cast<spdlog::level::level_enum>(level));
    if (tag) {
        // Simple global pattern with tag
        spdlog::set_pattern(fmt::format("[{}] [%Y-%m-%d %H:%M:%S.%e] [%l] %v", tag));
    }
}
