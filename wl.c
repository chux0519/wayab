#include "wl.h"
#include "render.h"

#include <stdlib.h>
#include <string.h>

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version) {
  struct wayab_wl *ptr = data;
  LOG("Got a registry event for %s id %d\n", interface, id);
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    ptr->compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    ptr->layer_shell =
        wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    struct wl_output *wl_output =
        wl_registry_bind(registry, id, &wl_output_interface, 3);
    struct wayab_renderer *renderer = wayab_renderer_new(wl_output, ptr);
    wl_list_insert(&ptr->renderers, &renderer->link);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id) {
  LOG("Got a registry losing event for %d\n", id);
}

static struct wl_registry_listener listener = {global_registry_handler,
                                               global_registry_remover};

struct wayab_wl *wayab_wl_new() {
  struct wayab_wl *ptr = calloc(1, sizeof(struct wayab_wl));
  wl_list_init(&ptr->renderers);

  ptr->display = wl_display_connect(NULL);
  if (ptr->display == NULL) {
    LOG("Can't connect to wayland display\n");
    goto error;
  }

  struct wl_registry *wl_registry = wl_display_get_registry(ptr->display);
  wl_registry_add_listener(wl_registry, &listener, ptr);
  wl_display_roundtrip(ptr->display);
  if (ptr->compositor == NULL || ptr->layer_shell == NULL) {
    LOG("No compositor\n");
    goto error;
  }

  return ptr;
error:
  if (ptr)
    wayab_wl_destroy(ptr);
  return NULL;
}

int wayab_wl_destroy(struct wayab_wl *ptr) {
  struct wayab_renderer *renderer, *tmp;
  wl_list_for_each_safe(renderer, tmp, &ptr->renderers, link) {
    wayab_renderer_destroy(renderer);
  }
  wl_display_disconnect(ptr->display);
  free(ptr);
  return 0;
}
