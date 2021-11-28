#include "image.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct wayab_image *wayab_image_new(const char *path, int width, int height) {
  struct wayab_image *ptr = calloc(1, sizeof(struct wayab_image));
  ptr->dir = strdup(path);
  ptr->count = 0;

  struct dirent *de;
  DIR *dir;

  dir = opendir(path);
  if (dir == NULL) {
    LOG("opendir");
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

  ptr->paths = malloc(ptr->count * sizeof(char *));
  ptr->patterns = malloc(ptr->count * sizeof(cairo_pattern_t *));

  dir = opendir(path);
  if (dir == NULL) {
    LOG("opendir");
    goto error;
  }
  /* store all paths */
  int n = 0;
  while ((de = readdir(dir)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
      continue;
    }
    int len = strlen(path) + 1 + strlen(de->d_name) + 1;
    ptr->paths[n] = malloc(len * sizeof(char));
    strcpy(ptr->paths[n], path);
    strcat(ptr->paths[n], "/");
    strcat(ptr->paths[n], de->d_name);
    ++n;
  }
  closedir(dir);

  /* sort in aplhabetical order */
  for (int i = 0; i < ptr->count; i++) {
    for (int j = i + 1; j < ptr->count; j++) {
      if (strcmp(ptr->paths[i], ptr->paths[j]) > 0) {
        char temp[256];
        strcpy(temp, ptr->paths[i]);
        strcpy(ptr->paths[i], ptr->paths[j]);
        strcpy(ptr->paths[j], temp);
      }
    }
  }

  /* TODO: load surface */
  for (int i = 0; i < ptr->count; i++) {
    LOG("%s\n", ptr->paths[i]);
    cairo_surface_t *surface =
        cairo_image_surface_create_from_png(ptr->paths[i]);
    ptr->patterns[i] = cairo_pattern_create_for_surface(surface);
    cairo_surface_destroy(surface);
  }

  return ptr;

error:
  if (dir)
    closedir(dir);
  if (ptr) {
    wayab_image_destroy(ptr);
  }

  return NULL;
}

int wayab_image_destroy(struct wayab_image *ptr) {
  if (ptr->dir)
    free(ptr->dir);
  if (ptr->paths) {
    for (int i = 0; i < ptr->count; ++i) {
      free(ptr->paths[i]);
    }
    free(ptr->paths);
  }
  if (ptr->patterns) {
    for (int i = 0; i < ptr->count; ++i) {
      cairo_pattern_destroy(ptr->patterns[i]);
    }
    free(ptr->patterns);
  }
  if (ptr)
    free(ptr);

  return 0;
}

void wayab_image_next_frame(struct wayab_image *ptr, uint64_t counter) {}
