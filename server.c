#include "server.h"

#include <stdio.h>
#include <ws2tcpip.h>


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

void parseArgv(int c, char **v, LaunchOption *lo) {
  if (c == 1) {
    printf("Choose an option:\r\n\t1. Server\r\n\t2. Client\r\n\t0. QUIT\r\n");
    *lo = getlaunchoption();
    return;
  } else if (c > 2)
    return;
  if (*v[1]++ != '-') {
    *lo = UNKNOWN_LAUNCH_OPTION;
    return;
  }
  switch (*v[1]) {
  case 's':
    *lo = SERVER;
    break;
  case 'c':
    *lo = CLIENT;
    break;
  default:
    *lo = UNKNOWN_LAUNCH_OPTION;
    return;
  }
}

LaunchOption getlaunchoption() { return getchar() - '0'; }

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

void setaddrinfo(AddrInfo *ai, LaunchOption lo) {
  ZeroMemory(ai, sizeof(*ai));
  (lo == SERVER) ? setaddrinfoserver(ai) : setaddrinfoclient(ai);
}

void setaddrinfoserver(AddrInfo *ai) {
  ai->ai_family = AF_INET;
  ai->ai_socktype = SOCK_STREAM;
  ai->ai_protocol = IPPROTO_TCP;
  ai->ai_flags = AI_PASSIVE;
}

void setaddrinfoclient(AddrInfo *ai) {
  ai->ai_family = AF_INET;
  ai->ai_socktype = SOCK_STREAM;
  ai->ai_protocol = IPPROTO_TCP;
}