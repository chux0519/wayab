#include "image.h"
#include "render.h"
#include "wl.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int FPS = 10;

static struct wayab_rule *wayab_rule_parse(const char *str) {
  int left, count;

  left = count = 0;

  struct wayab_rule *ptr = malloc(sizeof(struct wayab_rule));
  ptr->dir = NULL;
  ptr->output_name = NULL;
  ptr->resize = NONE;
  ptr->anchor_x = 0.5;
  ptr->anchor_y = 0.5;
  wl_list_init(&ptr->link);

  for (int i = 0; i < strlen(str); ++i) {
    if (str[i] == ':') {
      switch (count++) {
      case 0: {
        int output_len = i - left;
        ptr->output_name = malloc((output_len + 1) * sizeof(char));
        memcpy(ptr->output_name, str, output_len);
        ptr->output_name[output_len] = '\0';
        LOG("output_name: %s\n", ptr->output_name);
        break;
      }
      case 1: {
        int dir_len = i - left;
        ptr->dir = malloc((dir_len + 1) * sizeof(char));
        memcpy(ptr->dir, str + left, dir_len);
        ptr->dir[dir_len] = '\0';
        LOG("dir: %s\n", ptr->dir);
        break;
      }
      case 2: {
        int resize_len = i - left;
        char resize[resize_len + 1];
        memcpy(resize, str + left, resize_len);
        resize[resize_len] = '\0';
        LOG("resize: %s\n", resize);
        if (strcmp(resize, "none") == 0) {
          ptr->resize = NONE;
        }
        if (strcmp(resize, "fit") == 0) {
          ptr->resize = FIT;
        }
        if (strcmp(resize, "fill") == 0) {
          ptr->resize = FILL;
        }
        if (strcmp(resize, "stretch") == 0) {
          ptr->resize = STRETCH;
        }
        if (strcmp(resize, "tile") == 0) {
          ptr->resize = TILE;
        }
        // break;
      }
      case 3: {
        for (int j = i + 1; j < strlen(str); j++) {
          if (str[j] == ',') {
            int xlen = j - i - 1;
            int ylen = strlen(str) - j - 1;
            char x[xlen + 1];
            char y[ylen + 1];
            memcpy(x, str + i + 1, xlen);
            memcpy(y, str + i + 1 + xlen + 1, ylen);
            x[xlen] = '\0';
            y[ylen] = '\0';

            ptr->anchor_x = atof(x);
            ptr->anchor_y = atof(y);
            LOG("x(%d): %f, y(%d): %f\n", xlen, ptr->anchor_x, ylen,
                ptr->anchor_y);
          }
        }

        break;
      }
      default:
        break;
      }
      left = i + 1;
    }
  }

  if (count == 1) {
    int dir_len = strlen(str) - left;
    ptr->dir = malloc((dir_len + 1) * sizeof(char));
    memcpy(ptr->dir, str + left, dir_len);
    ptr->dir[dir_len] = '\0';
    LOG("dir: %s\n", ptr->dir);
  }

  if (ptr->dir == NULL || ptr->output_name == NULL) {
    goto error;
  }

  return ptr;
error:
  if (ptr) {
    if (ptr->dir)
      free(ptr->dir);
    if (ptr->output_name)
      free(ptr->output_name);
    free(ptr);
  }
  return NULL;
}

int main(int argc, char **argv) {
  struct wayab_config config = {0};
  config.fps = FPS;
  wl_list_init(&config.rules);

  int opt = 0;
  while ((opt = getopt(argc, argv, "hf:o:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: wayab -f <fps> -o <output>:<path>:<resize>:<anchor>\n");
      printf("\n");
      printf("-f: fps, default to %d.\n", FPS);
      printf("-o: for all monitors, pass `*` as output string.\n");
      return 0;
    case 'f':
      config.fps = atoi(optarg);
      break;
    case 'o': {
      struct wayab_rule *rule = wayab_rule_parse(optarg);
      if (rule) {
        wl_list_insert(&config.rules, &rule->link);
      }
      break;
    }
    }
  }

  struct wayab_wl *wl = wayab_wl_new(&config);
  if (wl == NULL) {
    LOG("wayab_wl_new");
    return -1;
  }

  wayab_wl_loop(wl);

  wayab_wl_destroy(wl);

  return 0;
}
