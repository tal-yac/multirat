#pragma once

#define LOCAL_HOST "127.0.0.1"
#define DEFAULT_PORT "8000"
#define MAX_CLIENTS 10
#define ADDR_LEN 16

#include "ratpacket.h"
#include "winapi.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

typedef struct addrinfo AddrInfo;
typedef struct sockaddr_in SocketAddress;

typedef enum options {
  UNKNOWN_LAUNCH_OPTION = -1,
  QUIT,
  SERVER,
  CLIENT
} LaunchOption;

typedef struct {
  SOCKET conn;
  SOCKET clients[MAX_CLIENTS];
  char clients_addrs[MAX_CLIENTS][ADDR_LEN];
} Server;

void setaddrinfo(AddrInfo *, LaunchOption);