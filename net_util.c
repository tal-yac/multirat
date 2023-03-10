#include "net_util.h"

void setaddrinfo(AddrInfo *ai) {
  ZeroMemory(ai, sizeof(*ai));
  ai->ai_family = AF_INET;
  ai->ai_socktype = SOCK_STREAM;
  ai->ai_protocol = IPPROTO_TCP;
  #ifdef SERVER_SIDE
  ai->ai_flags = AI_PASSIVE;
  #endif // SERVER_SIDE
}

void close_client(Client *client) {
  closesocket(client->conn);
  CloseHandle(client->handler);
  client->conn = INVALID_SOCKET;
}