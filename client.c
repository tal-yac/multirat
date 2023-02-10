#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ratpacket_t *readratpacket() {
  static char msg[DEFAULT_BUFLEN];
  msg[DEFAULT_BUFLEN - 1] = '\0';
  ratpacket_t *p = (ratpacket_t *)msg;
  printf("enter opcode: \n");
#define _(s, v, n) printf("%d. %s\n", ((v) + 1), n);
  FOREACH_RAT_OPCODE
#undef _
  fgets((char *)p, sizeof(p->op) + 2, stdin);
  p->op = atoi((char *)p) - 1;
  printf("enter data: ");
  fgets((char *)p->data, SIZE_OF_RAT_PACKET_DATA(msg) - 1, stdin);
  p->data_len = strlen((char *)p->data) + 1;
  return p;
}

static int sendratpacket(int debug, SOCKET conn, ratpacket_t *p) {
  if (send(conn, (char *)p, sizeof(ratpacket_t) + p->data_len, 0) ==
      SOCKET_ERROR) {
    if (debug)
      printf("send failed, error: %d\n", WSAGetLastError());
    closesocket(conn);
    return 1;
  }
  if (debug)
    printf("sent\n");
  return 0;
}

int client(int debug) {
  AddrInfo *result, *ptr, hints;
  result = ptr = NULL;
  setaddrinfo(&hints, CLIENT);
  if (getaddrinfo(LOCAL_HOST, DEFAULT_PORT, &hints, &result) != 0) {
    if (debug)
      printf("getaddrinfo failed, error: %d\n", WSAGetLastError());
    return 1;
  }
  SOCKET conn = INVALID_SOCKET;
  if (ptr == result) {
    if (debug)
      printf("equal\n");
  } else
    ptr = result;
  conn = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (conn == INVALID_SOCKET) {
    if (debug)
      printf("socket creating failed, error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    return 1;
  }
  if (connect(conn, result->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
    closesocket(conn);
    conn = INVALID_SOCKET;
  }
  freeaddrinfo(result);
  if (conn == INVALID_SOCKET) {
    if (debug)
      printf("error: unable to connect to the server\n");
    return 1;
  }
  char buf[DEFAULT_BUFLEN];
  ratpacket_t *p = (ratpacket_t *)buf;
  while (1) {
    ratpacket_t *puser = readratpacket();
    if (puser->op == RAT_PACKET_DISCONNECT)
      break;
    if (sendratpacket(debug, conn, puser))
      break;
    int recbytes = recv(conn, (char *)p, sizeof(ratpacket_t), 0);
    if (recbytes == 0) {
      if (debug)
        printf("connection closed");
      break;
    }
    if (recbytes < 0) {
      if (debug)
        printf("recv failed, error: %d\n", WSAGetLastError());
      break;
    }
    if (p->op == RAT_PACKET_ECHO) {
      recv(conn, (char *)p->data, p->data_len, 0);
      printf("%s", p->data);
    }
  }
  if (shutdown(conn, SD_SEND) == SOCKET_ERROR) {
    if (debug)
      printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(conn);
    return 1;
  }
  closesocket(conn);
  return 0;
}