#include <stdlib.h> /* getenv */
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include <set>
#include <unordered_map>

#include <sys/param.h>

#include "dobby/common.h"

#define LOG_TAG "FileOperationMonitor"

#include "./dobby_monitor.h"

std::unordered_map<FILE *, const char *> *TracedFopenFileList;

FILE *(*orig_fopen)(const char *filename, const char *mode);
FILE *fake_fopen(const char *filename, const char *mode) {
  FILE *result = NULL;
  result = orig_fopen(filename, mode);
  if (result != NULL) {
    char *traced_filename = (char *)malloc(MAXPATHLEN);
    // FIXME: strncpy
    strcpy(traced_filename, filename);
    INFO_LOG("[-] trace file: {}", filename);
    TracedFopenFileList->insert(std::make_pair(result, traced_filename));
  }
  return result;
}

static const char *GetFileDescriptorTraced(FILE *stream, bool removed) {
  std::unordered_map<FILE *, const char *>::iterator it;
  it = TracedFopenFileList->find(stream);
  if (it != TracedFopenFileList->end()) {
    if (removed)
      TracedFopenFileList->erase(it);
    return it->second;
  }
  return NULL;
}

size_t (*orig_fread)(void *ptr, size_t size, size_t count, FILE *stream);
size_t fake_fread(void *ptr, size_t size, size_t count, FILE *stream) {
  const char *file_name = GetFileDescriptorTraced(stream, false);
  if (file_name) {
    INFO_LOG("[-] fread: {}, buffer: {:p}", file_name, ptr);
  }
  return orig_fread(ptr, size, count, stream);
}

size_t (*orig_fwrite)(const void *ptr, size_t size, size_t count, FILE *stream);
size_t fake_fwrite(void *ptr, size_t size, size_t count, FILE *stream) {
  const char *file_name = GetFileDescriptorTraced(stream, false);
  if (file_name) {
    INFO_LOG("[-] fwrite: {}, from: {:p}", file_name, ptr);
  }
  return orig_fwrite(ptr, size, count, stream);
}

__attribute__((constructor)) void __main() {

  TracedFopenFileList = new std::unordered_map<FILE *, const char *>();

  //   DobbyHook((void *)fopen, (void *)fake_fopen, (void **)&orig_fopen);
  //   DobbyHook((void *)fwrite, (void *)fake_fwrite, (void **)&orig_fwrite);
  //   DobbyHook((void *)fread, (void *)fake_fread, (void **)&orig_fread);

  const char *temp_dir = getenv("TMPDIR");
  if (!temp_dir) temp_dir = "/data/local/tmp";

  std::string filePath = std::string(temp_dir) + "/dobby_temp.log";


  char buffer[64];
  memset(buffer, 'B', 64);

  FILE *fd = fopen(filePath.c_str(), "w+");
  if (!fd)
    INFO_LOG("[!] open {} failed!", filePath);

  fwrite(buffer, 64, 1, fd);
  fflush(fd);
  fseek(fd, 0, SEEK_SET);
  memset(buffer, 0, 64);

  fread(buffer, 64, 1, fd);

  return;
}
