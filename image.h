#ifndef _WAYAB_IMAGE_H
#define _WAYAB_IMAGE_H

#include "utils.h"

#include <cairo/cairo.h>
#include <stdint.h>

struct wayab_image {
  const struct wayab_rule *rule;
  int count;

  cairo_surface_t **surfaces;
};

struct wayab_image *wayab_image_new(const struct wayab_rule *, cairo_t *,
                                    int width, int height);
int wayab_image_destroy(struct wayab_image *);
int wayab_image_bootstrap(struct wayab_image *ptr, cairo_t *cr, int width,
                          int height, int bootstrap);
void wayab_image_next_frame(struct wayab_image *, uint64_t counter);

#endif
