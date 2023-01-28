#pragma once

#define _WIN32_WINNT 0x0601

#define LOCAL_HOST "127.0.0.1"
#define DEFAULT_PORT "8000"
#define DEFAULT_BUFLEN 1
#define DEFAULT_LINELEN 512

#include <ws2tcpip.h>

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