#pragma once

#include <stdint.h>

#define SIZE_OF_RAT_PACKET_DATA(p) (sizeof(p) - sizeof(ratpacket_t))

#define FOREACH_RAT_OPCODE                                                     \
  _(ECHO, 0, "Echo")                                                           \
  _(REGISTER_CLIENT, 1, "Register_client")                                     \
  _(UNREGISTER, 2, "Unregister")                                               \
  _(CONNECT_CLIENT, 3, "Connect_client")                                       \
  _(DISCONNECT, 4, "Disconnect")                                               \
  _(KEYLOG, 5, "Keylog")                                                       \
  _(TURN_OFF, 6, "Turn_off")                                                   \
  _(TURN_ON, 7, "Turn_on")                                                     \
  _(RESTART, 8, "Restart")                                                     \
  _(FILE, 9, "File")                                                           \
  _(REGISTRY, 10, "Registry")                                                  \
  _(CMD, 11, "Cmd")                                                            \
  _(ALERT, 12, "Alert")

typedef enum {
#define _(s, v, n) RAT_PACKET_##s = (v),
  FOREACH_RAT_OPCODE
#undef _
} op_codes;

typedef struct __attribute__((packed)) {
  uint8_t op;
  uint64_t data_len;
  uint8_t data[];
} ratpacket_t;

typedef struct {
  char *name;
  uint64_t content_len;
  uint8_t content[];
} entry_t;