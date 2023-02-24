#pragma once

#define DEFAULT_PORT "8000"
#define MAX_CLIENTS 10
#define ADDR_LEN 16

#include "ratpacket.h"
#include "winapi.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

typedef struct addrinfo AddrInfo;
typedef struct sockaddr_in SocketAddress;

typedef struct {
  SOCKET conn;
  char addr[ADDR_LEN];
} Client;

typedef struct {
  SOCKET conn;
  Client clients[MAX_CLIENTS];
} Server;

void setaddrinfo(AddrInfo *);

void close_client(SOCKET *client);