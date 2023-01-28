#include "net_util.h"

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