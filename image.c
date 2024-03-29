#include "image.h"
#include "utils.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

static void cairo_load(const char *path, int width, int height, double anchor_x,
                       double anchor_y, int mode, cairo_t *cr,
                       cairo_surface_t **frame) {

  cairo_surface_t *img_surface = cairo_image_surface_create_from_png(path);
  cairo_pattern_t *pattern = cairo_pattern_create_for_surface(img_surface);
  int s_width = cairo_image_surface_get_width(img_surface);
  int s_height = cairo_image_surface_get_height(img_surface);
  cairo_matrix_t matrix;
  cairo_matrix_init_identity(&matrix);
  // scale to fill output
  double scale_x = (double)width / (double)s_width;
  double scale_y = (double)height / (double)s_height;
  double scale;
  switch (mode) {
  case NONE:
    scale = 1.0;
    scale_x = scale;
    scale_y = scale;
    break;
  case FIT:
    scale = scale_x < scale_y ? scale_x : scale_y;
    scale_x = scale;
    scale_y = scale;
    break;
  case FILL:
    scale = scale_x > scale_y ? scale_x : scale_y;
    scale_x = scale;
    scale_y = scale;
    break;
  case STRETCH:
    break;
  case TILE:
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
    scale_x = 1;
    scale_y = 1;
    break;
  default:
    break;
  }

  double after_x = s_width * scale_x;
  double after_y = s_height * scale_y;

  // translate to center
  int offset_x = ((double)width - after_x) * anchor_x;
  int offset_y = ((double)height - after_y) * anchor_y;

  cairo_matrix_scale(&matrix, 1 / scale_x, 1 / scale_y);
  cairo_matrix_translate(&matrix, -offset_x, -offset_y);
  cairo_pattern_set_matrix(pattern, &matrix);

  cairo_push_group(cr);
  cairo_set_source(cr, pattern);
  cairo_paint(cr);

  cairo_surface_t *cache_surface =
      cairo_surface_reference(cairo_get_group_target(cr));
  *frame = cache_surface;

  cairo_pop_group_to_source(cr);

  cairo_pattern_destroy(pattern);
  cairo_surface_destroy(img_surface);
}

int wayab_image_bootstrap(struct wayab_image *ptr, cairo_t *cr, int width,
                          int height, int bootstrap) {
  char **paths = malloc(ptr->count * sizeof(char *));
  DIR *dir = NULL;

  dir = opendir(ptr->rule->dir);
  if (dir == NULL) {
    LOG("opendir failed\n");
    goto error;
  }

  /* store all paths */
  int n = 0;
  struct dirent *de;
  while ((de = readdir(dir)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
    int len = strlen(ptr->rule->dir) + 1 + strlen(de->d_name) + 1;
    paths[n] = malloc(len * sizeof(char));
    strcpy(paths[n], ptr->rule->dir);
    strcat(paths[n], "/");
    strcat(paths[n], de->d_name);
    ++n;
  }
  closedir(dir);

  /* sort in aplhabetical order */
  for (int i = 0; i < ptr->count; i++) {
    for (int j = i + 1; j < ptr->count; j++) {
      if (strcmp(paths[i], paths[j]) > 0) {
        char temp[256];
        strcpy(temp, paths[i]);
        strcpy(paths[i], paths[j]);
        strcpy(paths[j], temp);
      }
    }
  }

  if (ptr->count == 0)
    goto error;

  // after the first frame is loaded, we spawn a thread to do the rest to reduce
  // blank screen time
  if (bootstrap) {
    int start_frame = 0;
    LOG("Bootstrap using %s\n", paths[start_frame]);
    cairo_load(paths[start_frame], width, height, ptr->rule->anchor_x,
               ptr->rule->anchor_y, ptr->rule->resize, cr,
               &ptr->surfaces[start_frame]);
  } else {
    for (int i = 0; i < ptr->count; ++i) {
      LOG("Loading %s\n", paths[i]);
      cairo_load(paths[i], width, height, ptr->rule->anchor_x,
                 ptr->rule->anchor_y, ptr->rule->resize, cr, &ptr->surfaces[i]);
    }
  }

  return 0;

error:
  if (dir) {
    closedir(dir);
  }
  if (paths) {
    for (int i = 0; i < ptr->count; ++i) {
      if (paths[i])
        free(paths[i]);
    }
    free(paths);
  }
  return -1;
}

struct wayab_image *wayab_image_new(const struct wayab_rule *rule, cairo_t *cr,
                                    int width, int height) {
  struct wayab_image *ptr = calloc(1, sizeof(struct wayab_image));
  ptr->rule = rule;
  ptr->count = 0;

  struct dirent *de;
  DIR *dir;

  dir = opendir(rule->dir);
  if (dir == NULL) {
    LOG("opendir failed\n");
    goto error;
  }

  /* count frames */
  while ((de = readdir(dir)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
    ++ptr->count;
  }
  closedir(dir);

  /* init surface */
  ptr->surfaces = malloc(ptr->count * sizeof(cairo_surface_t *));
  for (int i = 0; i < ptr->count; ++i)
    ptr->surfaces[i] = NULL;

  if (wayab_image_bootstrap(ptr, cr, width, height, 1))
    goto error;

  return ptr;
error:
  if (dir)
    closedir(dir);
  if (ptr)
    wayab_image_destroy(ptr);

  return NULL;
}

int wayab_image_destroy(struct wayab_image *ptr) {
  if (ptr->surfaces) {
    for (int i = 0; i < ptr->count; ++i) {
      cairo_surface_destroy(ptr->surfaces[i]);
    }
    free(ptr->surfaces);
  }
  if (ptr)
    free(ptr);

  return 0;
}

void wayab_image_next_frame(struct wayab_image *ptr, uint64_t counter) {}
