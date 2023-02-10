#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int readandsend(int debug, SOCKET conn) {
  static char msg[DEFAULT_BUFLEN];
  msg[DEFAULT_BUFLEN - 1] = '\0';
  ratpacket_t *p = (ratpacket_t *)msg;
  printf("enter opcode: ");
  fgets((char *)p, sizeof(p->op) + 2, stdin);
  p->op = atoi((char *)p);
  printf("enter message: ");
  fgets((char *)p->data, SIZE_OF_RAT_PACKET_DATA(msg) - 1, stdin);
  p->data_len = strlen((char *)p->data) + 1;
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
    if (readandsend(debug, conn))
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
    if (p->op == echo) {
      recv(conn, (char *)p->data, p->data_len, 0);
      printf("%s", p->data);
    }
    if (strcmp((char *)p->data, "exit\n") == 0)
      break;
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