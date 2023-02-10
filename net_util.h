#pragma once

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#define LOCAL_HOST "127.0.0.1"
#define DEFAULT_PORT "8000"
#define DEFAULT_BUFLEN 64

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
void setaddrinfoserver(AddrInfo *);
void setaddrinfoclient(AddrInfo *);