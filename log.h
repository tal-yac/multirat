#pragma once

#include <stdio.h>

#define LOG_FILE stdout

#define LOG_INFO 0
#define LOG_DEBUG 1

#define LOG(f, ...)                                                            \
  fprintf(LOG_FILE, "%s::%s %d " f "\n", __FILE__, __FUNCTION__, __LINE__,    \
          ##__VA_ARGS__)

#if LOG_LEVEL >= LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(f, ...) LOG(f, ##__VA_ARGS__)
#else
#undef LOG_DEBUG
#define LOG_DEBUG(f, ...)
#endif // LOG_LEVEL >= LOG_DEBUG

#define LOG_ERR(f, ...) LOG("error: " f, ##__VA_ARGS__)