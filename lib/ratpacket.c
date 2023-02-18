#include "ratpacket.h"

char *rat_opcode_to_str(op_codes op) {
  switch (op) {
#define _(s, v, n)                                                             \
  case (v):                                                                    \
    return (n);
    FOREACH_RAT_OPCODE
#undef _
  default:
    return "unknown";
  }
}