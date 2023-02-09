#include "server.h"

#include <stdio.h>


int server() {
  // creating the server socket
  SOCKET server, client;
  struct addrinfo hints, *result;
  setaddrinfo(&hints, SERVER);
  result = NULL;
  if (getaddrinfo(NULL, DEFAULT_PORT, &hints, &result) != 0) {
    printf("error: %d\n", WSAGetLastError());
    return 1;
  }
  server = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (server == INVALID_SOCKET) {
    printf("error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    return 1;
  }
  // binding the socket
  if (bind(server, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
    printf("bind failed, error: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(server);
    return 1;
  }
  freeaddrinfo(result);
  // listening
  printf("listening...\n");
  if (listen(server, SOMAXCONN) == SOCKET_ERROR) {
    printf("listen failed, error: %d\n", WSAGetLastError());
    closesocket(server);
    return 1;
  }
  // accepting a connection
  printf("accetping...\n");
  client = accept(server, NULL, NULL);
  if (client == INVALID_SOCKET) {
    printf("accept failed, error: %d", WSAGetLastError());
    closesocket(server);
    return 1;
  }
  printf("accepted\n");
  closesocket(server);
  // communicate
  int sentbytes, iline;
  int recvbuflen = DEFAULT_BUFLEN;
  char recvbuf[DEFAULT_BUFLEN];
  char line[DEFAULT_LINELEN];
  for (int i = 0; i < recvbuflen; i++)
    recvbuf[i] = 0;
  for (int i = iline = 0; i < DEFAULT_LINELEN; i++)
    line[i] = 0;
  do {
    sentbytes = recv(client, recvbuf, recvbuflen, 0);
    if (sentbytes > 0) {
      line[iline++] = recvbuf[0];
      if (recvbuf[0] == '\r' || iline == DEFAULT_LINELEN) {
        printf("%s\n", line);
        fflush(stdout);
        if (send(client, recvbuf, recvbuflen, 0) == SOCKET_ERROR) {
          printf("send failed, error: %d", WSAGetLastError());
          closesocket(client);
          return 1;
        }
        printf("echo completed\n");
        for (int i = iline = 0; i < iline; i++)
          line[i] = 0;
        sentbytes = 0;
      }
    } else if (sentbytes == 0)
      printf("closing connection...");
    else {
      printf("recv failed, error: %d", WSAGetLastError());
      closesocket(client);
      return 1;
    }
  } while (sentbytes > 0);
  if (shutdown(client, SD_SEND) == SOCKET_ERROR) {
    printf("shutdown failed, error: %d", WSAGetLastError());
    closesocket(client);
    return 1;
  }
  closesocket(client);
  return 0;
}