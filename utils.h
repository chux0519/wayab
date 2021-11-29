#ifndef _WAYAB_UTILS_H
#define _WAYAB_UTILS_H

#include <wayland-util.h>

#include <stdio.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERRNO(...)                                                         \
  fprintf(stderr, "Error : %s\n", strerror(errno));                            \
  fprintf(stderr, __VA_ARGS__)

enum resize_mode { NONE = 0, FILL, FIT, STRETCH, TILE };

struct wayab_rule {
  char *output_name;
  char *dir;

  enum resize_mode resize;
  double anchor_x;
  double anchor_y;

  struct wl_list link;
};

struct wayab_config {
  int fps;

  struct wl_list rules;
};

#endif
