#include "client.h"

#include <stdio.h>

int client() {
  AddrInfo *result, *ptr, hints;
  result = ptr = NULL;
  setaddrinfo(&hints, CLIENT);
  if (getaddrinfo(LOCAL_HOST, DEFAULT_PORT, &hints, &result) != 0) {
    printf("getaddrinfo failed, error: %d\n", WSAGetLastError());
    return 1;
  }
  SOCKET conn = INVALID_SOCKET;
  if (ptr == result)
    printf("equal\n");
  else
    ptr = result;
  conn = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
  if (conn == INVALID_SOCKET) {
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
    printf("error: unable to connect to the server\n");
    return 1;
  }
  char msg[] = "hello this is client\r\n";
  char buf[DEFAULT_BUFLEN];
  if (send(conn, msg, (int)strlen(msg), 0) == SOCKET_ERROR) {
    printf("send failed, error: %d\n", WSAGetLastError());
    closesocket(conn);
    return 1;
  }
  printf("sent");
  int recbytes;
  do {
    recbytes = recv(conn, buf, DEFAULT_BUFLEN, 0);
    if (recbytes > 0) {
      printf("%s\n", buf);
      break;
    } else if (recbytes == 0) {
      printf("connection closed");
    } else {
      printf("recv failed, error: %d\n", WSAGetLastError());
    }
  } while (recbytes > 0);
  if (shutdown(conn, SD_SEND) == SOCKET_ERROR) {
    printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(conn);
    return 1;
  }
  closesocket(conn);
  return 0;
}