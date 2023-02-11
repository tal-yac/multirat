#pragma once

#include <stdio.h>

#define LOG_FILE stdout

#define LOG_INFO 0
#define LOG_DEBUG 1

#if LOG_LEVEL >= LOG_DEBUG
#define LOG(s, f, ...)                                                         \
  fprintf(LOG_FILE, "%s::%s %d: " f "\n", __FILE__, __FUNCTION__, __LINE__,    \
          ##__VA_ARGS__)
#else
#define LOG(s, f, ...)
#endif // LOG_LEVEL >= LOG_DEBUG
