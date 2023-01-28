#include "net_util.h"
#include "WS2tcpip.h"
#include "stdio.h"
#include "server.h"
#include "client.h"

LaunchOption getlaunchoption() { return getchar() - '0'; }

int debug = 0;

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
  switch (*v[1]++) {
  case 's':
    *lo = SERVER;
    break;
  case 'c':
    *lo = CLIENT;
    if (*v[1] == 'd')
      debug = 1;
    break;
  default:
    *lo = UNKNOWN_LAUNCH_OPTION;
    return;
  }
}

int main(int argc, char **argv) {
  // initializing winsock
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    printf("init error: %d\n", WSAGetLastError());
    return 1;
  }
  LaunchOption lo;
  parseArgv(argc, argv, &lo);
  int r;
  switch (lo) {
  case QUIT:
    r = 0;
    break;
  case SERVER:
    r = server();
    break;
  case CLIENT:
    r = client(debug);
    break;
  case UNKNOWN_LAUNCH_OPTION:
  default:
    printf("error: unknown option, program will exit.");
    r = 1;
  }
  fflush(stdout);
  WSACleanup();
  return r;
}