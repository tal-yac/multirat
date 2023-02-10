#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

static SOCKET acceptclient() {
  AddrInfo hints;
  AddrInfo *result = NULL;
  setaddrinfo(&hints, SERVER);
  if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
    printf("error: %d\n", WSAGetLastError());
    return INVALID_SOCKET;
  }
  SOCKET server =
      socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (server == INVALID_SOCKET) {
    printf("error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    return INVALID_SOCKET;
  }
  // binding the socket
  if (bind(server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
    printf("bind failed, error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(server);
    return INVALID_SOCKET;
  }
  freeaddrinfo(result);
  result = NULL;
  // listening
  printf("listening...\n");
  if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
    printf("listen failed, error: %d\n", WSAGetLastError());
    closesocket(server);
    return INVALID_SOCKET;
  }
  // accepting a connection
  printf("accetping...\n");
  SOCKET client = accept(server, NULL, NULL);
  if (client == INVALID_SOCKET) {
    printf("accept failed, error: %d", WSAGetLastError());
    closesocket(server);
    return INVALID_SOCKET;
  }
  printf("accepted\n");
  closesocket(server);
  return client;
}

int server() {
  // creating the server socket
  SOCKET client = acceptclient();
  // communicate
  char buf[DEFAULT_BUFLEN];
  memset(buf, 0, DEFAULT_BUFLEN);
  ratpacket_t *p = (ratpacket_t *)buf;
  while (1) {
    int sentbytes = recv(client, buf, sizeof(ratpacket_t), 0);
    if (sentbytes == 0) {
      printf("closing connection...");
      break;
    }
    if (sentbytes < 0) {
      printf("recv failed, error: %d", WSAGetLastError());
      closesocket(client);
      return 1;
    }
    if (p->op == echo) {
      sentbytes = recv(client, (char *) p->data, p->data_len, 0);
      printf("%s\n", p->data);
      fflush(stdout);
      if (send(client,  (char *) p, sizeof(*p) + p->data_len, 0) == SOCKET_ERROR) {
        printf("send failed, error: %d", WSAGetLastError());
        closesocket(client);
        return 1;
      }
      printf("echo completed\n");
    }
  }
  if (shutdown(client, SD_SEND) == SOCKET_ERROR) {
    printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(client);
    return 1;
  }
  closesocket(client);
  return 0;
}