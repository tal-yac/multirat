#include <stdint.h>

typedef enum {
  register_client,
  unregister,
  connect,
  disconnect,
  keylog,
  turn_off,
  turn_on,
  restart,
  file,
  registry,
  cmd,
  alert
} op_codes;

typedef struct __attribute__((packed)) {
  uint8_t op;
  uint64_t data_len;
  uint8_t data[];
} ratpakcet_t;

typedef struct {
  char *name;
  uint64_t content_len;
  uint8_t content[];
} entry_t;
