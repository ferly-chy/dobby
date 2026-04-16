#pragma once

#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "dobby/common.h"

namespace dobby {

class ThreadFreezer {
public:
  ThreadFreezer() : frozen_(false) {
    // Pre-allocate vector to avoid malloc during freeze
    threads_.reserve(128);
  }
  
  ~ThreadFreezer() { ResumeAll(); }

  bool FreezeAll() {
    if (frozen_) return true;

    pid_t pid = getpid();
    pid_t tid = gettid();

    // Use opendir/readdir BEFORE we start stopping threads to minimize deadlock risk
    DIR* dir = opendir("/proc/self/task");
    if (!dir) return false;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      if (entry->d_name[0] == '.') continue;

      pid_t t = (pid_t)atoi(entry->d_name);
      if (t == tid) continue;

      threads_.push_back(t);
    }
    closedir(dir);

    // Stop all collected threads
    for (pid_t t : threads_) {
      if (tgkill(pid, t, SIGSTOP) == 0) {
        // Wait for thread to actually stop (state 'T')
        WaitForThreadStop(t);
      }
    }

    frozen_ = true;
    return true;
  }

  void ResumeAll() {
    if (!frozen_) return;

    pid_t pid = getpid();
    for (pid_t t : threads_) {
      tgkill(pid, t, SIGCONT);
    }
    threads_.clear();
    frozen_ = false;
  }

private:
  void WaitForThreadStop(pid_t tid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/self/task/%d/stat", tid);
    
    // Poll for stopped state 'T'
    for (int i = 0; i < 1000; ++i) {
      int fd = open(path, O_RDONLY);
      if (fd < 0) break;
      
      char buf[256];
      ssize_t n = read(fd, buf, sizeof(buf) - 1);
      close(fd);
      
      if (n > 0) {
        buf[n] = '\0';
        char *state_ptr = strrchr(buf, ')'); // find end of comm field
        if (state_ptr && state_ptr[1] == ' ' && state_ptr[2] == 'T') {
          return; // Thread is stopped
        }
      }
      usleep(100); // 0.1ms
    }
    // Timeout - continue anyway
  }

  bool frozen_;
  std::vector<pid_t> threads_;
};

} // namespace dobby
