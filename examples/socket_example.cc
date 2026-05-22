#include "dobby.h"
#include "logging/logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include <iostream>
#include <map>
#include <vector>

std::map<void *, const char *> *func_map;

// clang-format off
const char *func_array[] = {
  "dlsym",
  "dlclose",
  "open",
  "write",
  "read",
  "close",
  "socket",
  "connect",
  "bind",
  "listen",
  "accept",
  "send",
  "recv",
};

const char *func_short_array[] = {
  "accept",
};
// clang-format on

#define install_hook(name, fn_ret_t, fn_args_t...)                                                                     \
  fn_ret_t (*orig_##name)(fn_args_t);                                                                                  \
  fn_ret_t fake_##name(fn_args_t);                                                                                     \
  static void install_hook_##name() {                                                                                  \
    void *sym_addr = DobbySymbolResolver(NULL, #name);                                                                 \
    DobbyHook(sym_addr, (dobby_func_t)fake_##name, (dobby_func_t *)&orig_##name);                          \
    printf("install hook %s:%p:%p\n", #name, sym_addr, (void *)orig_##name);                                                   \
  }                                                                                                                    \
  fn_ret_t fake_##name(fn_args_t)

install_hook(pthread_create, int, pthread_t *thread, const pthread_attr_t *attrs, void *(*start_routine)(void *),
             void *arg) {
  INFO_LOG("pthread_create: {:p}", (void *)start_routine);
  return orig_pthread_create(thread, attrs, start_routine, arg);
}

void common_handler(void *address, DobbyRegisterContext *ctx) {
  auto iter = func_map->find(address);
  if (iter != func_map->end()) {
    INFO_LOG("func {}:{:p} invoke", iter->second, (void *)iter->first);
  }
}

uint64_t socket_demo_server(void *ctx);
uint64_t socket_demo_client(void *ctx);

__attribute__((constructor)) static void ctor() {
  logger_set_options(0, 0, 0, LOG_LEVEL_DEBUG, false, false);

  void *func = NULL;
  func_map = new std::map<void *, const char *>();
  for (int i = 0; i < sizeof(func_array) / sizeof(char *); ++i) {
    func = DobbySymbolResolver(NULL, func_array[i]);
    if (func == NULL) {
      INFO_LOG("func {} not resolve", func_array[i]);
      continue;
    }
    func_map->insert(std::pair<void *, const char *>(func, func_array[i]));
  }

  for (auto iter = func_map->begin(), e = func_map->end(); iter != e; iter++) {
    bool is_short = false;
    for (int i = 0; i < sizeof(func_short_array) / sizeof(char *); ++i) {
      if (strcmp(func_short_array[i], iter->second) == 0) {
        is_short = true;
        break;
      }
    }
    if (is_short) {
      dobby_enable_near_branch_trampoline();
      DobbyInstrument(iter->first, common_handler);
      dobby_disable_near_branch_trampoline();
    } else {
      DobbyInstrument(iter->first, common_handler);
    }
  }

  pthread_t socket_server;
  pthread_create(&socket_server, NULL, (void *(*)(void *))socket_demo_server, NULL);

  usleep(10000);
  pthread_t socket_client;
  pthread_create(&socket_client, NULL, (void *(*)(void *))socket_demo_client, NULL);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 49494

uint64_t socket_demo_server(void *ctx) {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[1024] = {0};
  const char *hello = "Hello from server";

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    ERROR_LOG("socket failed: {}", strerror(errno));
    return -1;
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    ERROR_LOG("setsockopt: {}", strerror(errno));
    return -1;
  }

  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    ERROR_LOG("bind failed: {}", strerror(errno));
    return -1;
  }
  if (listen(server_fd, 3) < 0) {
    ERROR_LOG("listen failed: {}", strerror(errno));
    return -1;
  }
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
    ERROR_LOG("accept failed: {}", strerror(errno));
    return -1;
  }

  int ret = recv(new_socket, buffer, 1024, 0);
  INFO_LOG("[server] {}", buffer);

  send(new_socket, hello, strlen(hello), 0);
  INFO_LOG("[server] Hello message sent");
  return 0;
}

uint64_t socket_demo_client(void *ctx) {
  int sock = 0;
  struct sockaddr_in serv_addr;
  const char *hello = "Hello from client";
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    ERROR_LOG("socket failed");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    ERROR_LOG("inet_pton failed");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    ERROR_LOG("connect failed");
    return -1;
  }

  send(sock, hello, strlen(hello), 0);
  INFO_LOG("[client] Hello message sent");

  int ret = recv(sock, buffer, 1024, 0);
  INFO_LOG("[client] {}", buffer);
  return 0;
}
