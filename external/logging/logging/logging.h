#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <fmt/format.h>
#include <string>
#endif

#define LOG_TAG NULL

typedef enum {
  LOG_LEVEL_DEBUG = 0,
  LOG_LEVEL_INFO = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_ERROR = 3,
  LOG_LEVEL_FATAL = 4
} LogLevel;

#ifdef __cplusplus

#if defined(USE_CXX_FILESTREAM)
#include <fstream>
#endif

class Logger {
public:
  LogLevel log_level_;

  const char *log_tag_;

  const char *log_file_;
#if defined(USE_CXX_FILESTREAM)
  std::fstream *log_file_stream_;
#else
  FILE *log_file_stream_;
#endif

  bool enable_time_tag_;
  bool enable_syslog_;

  static Logger *g_logger;
  static Logger *Shared() {
    if (g_logger == nullptr) {
      g_logger = new Logger();
    }
    return g_logger;
  }

  Logger() {
    log_tag_ = nullptr;
    log_file_ = nullptr;
    log_level_ = LOG_LEVEL_DEBUG;
    enable_time_tag_ = false;
    enable_syslog_ = false;
  }

  Logger(const char *tag, const char *file, LogLevel level, bool enable_time_tag, bool enable_syslog) {
    setTag(tag);
    setLogFile(file);
    setLogLevel(level);
    enable_time_tag_ = enable_time_tag;
    enable_syslog_ = enable_syslog;
  }

  void setOptions(const char *tag, const char *file, LogLevel level, bool enable_time_tag, bool enable_syslog) {
    if (tag)
      setTag(tag);
    if (file)
      setLogFile(file);
    setLogLevel(level);
    enable_time_tag_ = enable_time_tag;
    enable_syslog_ = enable_syslog;
  }

  void setTag(const char *tag) {
    log_tag_ = tag;
  }

  void setLogFile(const char *file) {
    log_file_ = file;
#if defined(USE_CXX_FILESTREAM)
    log_file_stream_ = new std::fstream();
    log_file_stream_->open(log_file_, std::ios::out | std::ios::app);
#else
    log_file_stream_ = fopen(log_file_, "a");
#endif
  }

  void setLogLevel(LogLevel level) {
    log_level_ = level;
  }

  void enableTimeTag() {
    enable_time_tag_ = true;
  }

  void enableSyslog() {
    enable_syslog_ = true;
  }

  void logv(LogLevel level, const char *fmt, va_list ap);

  template <typename... Args>
  void log(LogLevel level, fmt::format_string<Args...> format, Args &&...args) {
    if (level < log_level_)
      return;
    std::string s = fmt::format(format, std::forward<Args>(args)...);
    log_internal(level, s.c_str());
  }

  template <typename... Args>
  void debug(fmt::format_string<Args...> format, Args &&...args) {
    log(LOG_LEVEL_DEBUG, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void info(fmt::format_string<Args...> format, Args &&...args) {
    log(LOG_LEVEL_INFO, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void warn(fmt::format_string<Args...> format, Args &&...args) {
    log(LOG_LEVEL_WARN, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void error(fmt::format_string<Args...> format, Args &&...args) {
    log(LOG_LEVEL_ERROR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void fatal(fmt::format_string<Args...> format, Args &&...args) {
    log(LOG_LEVEL_FATAL, format, std::forward<Args>(args)...);
  }

  void log_internal(LogLevel level, const char *msg);
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(DOBBY_LOGGING_DISABLE)
#define LOG_FUNCTION_IMPL(...)
#else
#if !defined(LOG_FUNCTION_IMPL)
#define LOG_FUNCTION_IMPL logger_log_impl
#endif
#endif

void *logger_create(const char *tag, const char *file, LogLevel level, bool enable_time_tag, bool enable_syslog);
void logger_set_options(void *logger, const char *tag, const char *file, LogLevel level, bool enable_time_tag,
                        bool enable_syslog);
void logger_log_impl(void *logger, LogLevel level, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define LOG(level, format, ...)                                                                                        \
  do {                                                                                                                 \
    if (LOG_TAG)                                                                                                       \
      Logger::Shared()->log(level, "[{}] " format, LOG_TAG, ##__VA_ARGS__);                                            \
    else                                                                                                               \
      Logger::Shared()->log(level, format, ##__VA_ARGS__);                                                             \
  } while (0)
#else
#define LOG(level, fmt, ...)                                                                                           \
  do {                                                                                                                 \
    if (LOG_TAG)                                                                                                       \
      LOG_FUNCTION_IMPL(NULL, level, "[%s] " fmt, LOG_TAG, ##__VA_ARGS__);                                             \
    else                                                                                                               \
      LOG_FUNCTION_IMPL(NULL, level, fmt, ##__VA_ARGS__);                                                              \
  } while (0)
#endif

#define DEBUG_LOG(fmt, ...)                                                                                            \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__);                                                                          \
  } while (0)

#define INFO_LOG(fmt, ...)                                                                                             \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__);                                                                           \
  } while (0)

#define WARN_LOG(fmt, ...)                                                                                             \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__);                                                                           \
  } while (0)

#ifdef __cplusplus
#define ERROR_LOG(fmt, ...)                                                                                            \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_ERROR, "[!] [{}:{}:{}]" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);                           \
  } while (0)

#define FATAL_LOG(fmt, ...)                                                                                            \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_FATAL, "[!] [{}:{}:{}]" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);                           \
  } while (0)
#else
#define ERROR_LOG(fmt, ...)                                                                                            \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_ERROR, "[!] [%s:%d:%s]" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);                           \
  } while (0)

#define FATAL_LOG(fmt, ...)                                                                                            \
  do {                                                                                                                 \
    LOG(LOG_LEVEL_FATAL, "[!] [%s:%d:%s]" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);                           \
  } while (0)
#endif

#define UNIMPLEMENTED() FATAL_LOG("{}\n", "unimplemented code!!!")
#define UNREACHABLE() FATAL_LOG("{}\n", "unreachable code!!!")
