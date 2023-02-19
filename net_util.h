#pragma once

#define LOCAL_HOST "127.0.0.1"
#define DEFAULT_PORT "8000"

#include "winapi.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "ratpacket.h"

typedef struct addrinfo AddrInfo;

typedef enum options {
  UNKNOWN_LAUNCH_OPTION = -1,
  QUIT,
  SERVER,
  CLIENT
} LaunchOption;

void setaddrinfo(AddrInfo *, LaunchOption);