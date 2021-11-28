#ifndef _WAYAB_UTILS_H
#define _WAYAB_UTILS_H

#include <stdio.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERRNO(...)                                                         \
  fprintf(stderr, "Error : %s\n", strerror(errno));                            \
  fprintf(stderr, __VA_ARGS__)

#endif
