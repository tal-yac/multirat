#include "net_util.h"

static void setaddrinfoserver(AddrInfo *ai) {
  ai->ai_family = AF_INET;
  ai->ai_socktype = SOCK_STREAM;
  ai->ai_protocol = IPPROTO_TCP;
  ai->ai_flags = AI_PASSIVE;
}

static void setaddrinfoclient(AddrInfo *ai) {
  ai->ai_family = AF_INET;
  ai->ai_socktype = SOCK_STREAM;
  ai->ai_protocol = IPPROTO_TCP;
}

void setaddrinfo(AddrInfo *ai, LaunchOption lo) {
  ZeroMemory(ai, sizeof(*ai));
  (lo == SERVER) ? setaddrinfoserver(ai) : setaddrinfoclient(ai);
}

void close_client(SOCKET *client) {
  closesocket(*client);
  *client = INVALID_SOCKET;
}