#include "image.h"
#include "render.h"
#include "wl.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int FPS = 10;

static struct wayab_rule *wayab_rule_parse(const char *str) {
  int pos = 0;
  for (int i = 0; i < strlen(str); ++i) {
    if (str[i] == ':') {
      pos = i;
      break;
    }
  }
  if (pos > 0) {
    struct wayab_rule *ptr = malloc(sizeof(struct wayab_rule));
    wl_list_init(&ptr->link);

    int output_len = pos;
    ptr->output_name = malloc((output_len + 1) * sizeof(char));
    memcpy(ptr->output_name, str, output_len);
    ptr->output_name[output_len] = '\0';

    int dir_len = strlen(str) - pos - 1;
    ptr->dir = malloc((dir_len + 1) * sizeof(char));
    memcpy(ptr->dir, str + pos + 1, dir_len);
    ptr->dir[dir_len] = '\0';

    return ptr;
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
      printf("Usage: wayab -f <fps> -o <output>:</path/to/picture/dir>\n");
      printf("\n");
      printf("-f: fps, default to %d.\n", FPS);
      printf("-o: for all monitors, pass `*:</path/to/picture/dir>`.\n");
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
