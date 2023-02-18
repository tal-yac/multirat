#undef UNICODE

#include "net_util.h"
#include "stdio.h"
#include "client.h"

int __cdecl main(int argc, char **argv) {
  // initializing winsock
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("init error: %d\n", WSAGetLastError());
    return 1;
  }
  client();
  WSACleanup();
  return 0;
}