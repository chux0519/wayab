#ifndef _WAYAB_IMAGE_H
#define _WAYAB_IMAGE_H

#include "utils.h"

#include <cairo/cairo.h>
#include <stdint.h>

struct wayab_image {
  char *dir; // dir
  int count;

  char **paths;
  cairo_pattern_t **patterns;
};

struct wayab_image *wayab_image_new(const char *path, int width, int height);
int wayab_image_destroy(struct wayab_image *);
void wayab_image_next_frame(struct wayab_image *, uint64_t counter);

#endif
