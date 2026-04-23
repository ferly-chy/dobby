#include "logging/logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#endif

#if defined(__APPLE__)
#include <dlfcn.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/sysctl.h>
#include <sys/socket.h>
#include "./priv_headers/_simple.h"
#endif

#if defined(_WIN32)
#define PUBLIC
#else
#define PUBLIC __attribute__((visibility("default")))
#define INTERNAL __attribute__((visibility("internal")))
#endif

#if defined(__ANDROID__)
#include <android/log.h>
#endif

#pragma clang diagnostic ignored "-Wformat"

Logger *Logger::g_logger = nullptr;

void Logger::log_internal(LogLevel level, const char *msg) {
  if (level < log_level_)
    return;

  char fmt_buffer[4096] = {0};
  int offset = 0;

  if (enable_time_tag_) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    offset += snprintf(fmt_buffer + offset, sizeof(fmt_buffer) - offset, "%04d-%02d-%02d %02d:%02d:%02d ",
                       tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  }

  snprintf(fmt_buffer + offset, sizeof(fmt_buffer) - offset, "%s\n", msg);

  if (enable_syslog_) {
#if defined(__APPLE__)
    vsyslog(LOG_ALERT, "%s", (va_list)fmt_buffer); // This is not quite right for vsyslog but we'll fix it if needed

    static int _logDescriptor = 0;
    if (_logDescriptor == 0) {
      _logDescriptor = socket(AF_UNIX, SOCK_DGRAM, 0);
      if (_logDescriptor != -1) {
        fcntl(_logDescriptor, F_SETFD, FD_CLOEXEC);
        struct sockaddr_un addr;
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, _PATH_LOG, sizeof(addr.sun_path));
        if (connect(_logDescriptor, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
          close(_logDescriptor);
          _logDescriptor = -1;
          // We can't use ERROR_LOG here as it might cause recursion
        }
      }
    }
    if (_logDescriptor > 0) {
      dprintf(_logDescriptor, "%s", fmt_buffer);
    }
#elif defined(_POSIX_VERSION)
    syslog(LOG_ERR, "%s", fmt_buffer);
#endif
  }

  if (log_file_ != nullptr) {
#if defined(USER_CXX_FILESTREAM)
    log_file_stream_->write(fmt_buffer, strlen(fmt_buffer));
    log_file_stream_->flush();
#else
    fwrite(fmt_buffer, strlen(fmt_buffer), 1, log_file_stream_);
    fflush(log_file_stream_);
#endif
  }

#if defined(__ANDROID__)
  __android_log_print(ANDROID_LOG_INFO, "Dobby", "%s", fmt_buffer);
#else
  printf("%s", fmt_buffer);
#endif
}

void Logger::logv(LogLevel level, const char *_fmt, va_list ap) {
  char buffer[0x4000] = {0};
  vsnprintf(buffer, sizeof(buffer) - 1, _fmt, ap);
  log_internal(level, buffer);
}

#pragma clang diagnostic warning "-Wformat"

void *logger_create(const char *tag, const char *file, LogLevel level, bool enable_time_tag, bool enable_syslog) {
  Logger *logger = new Logger(tag, file, level, enable_time_tag, enable_syslog);
  return logger;
}

void logger_set_options(void *logger, const char *tag, const char *file, LogLevel level, bool enable_time_tag,
                        bool enable_syslog) {
  if (logger == nullptr) {
    logger = Logger::Shared();
  }
  ((Logger *)logger)->setOptions(tag, file, level, enable_time_tag, enable_syslog);
}

void logger_log_impl(void *logger, LogLevel level, const char *fmt, ...) {
  if (logger == nullptr) {
    logger = Logger::Shared();
  }
  va_list ap;
  va_start(ap, fmt);
  ((Logger *)logger)->logv(level, fmt, ap);
  va_end(ap);
}
