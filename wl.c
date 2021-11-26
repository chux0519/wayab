#include "wl.h"

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
    ptr->output = wl_registry_bind(registry, id, &wl_output_interface, 3);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id) {
  LOG("Got a registry losing event for %d\n", id);
}

static struct wl_registry_listener listener = {global_registry_handler,
                                               global_registry_remover};

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  struct wayab_wl *ptr = data;
  struct wl_region *opaque = wl_compositor_create_region(ptr->compositor);
  wl_region_add(opaque, 0, 0, width, height);
  wl_surface_set_opaque_region(ptr->surface, opaque);
  wl_region_destroy(opaque);

  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface
                                 __attribute__((unused))) {}

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

struct wayab_wl *wayab_wl_new() {
  struct wayab_wl *ptr = calloc(1, sizeof(struct wayab_wl));

  ptr->display = wl_display_connect(NULL);
  if (ptr->display == NULL) {
    LOG("Can't connect to wayland display\n");
    goto error;
  }

  struct wl_registry *wl_registry = wl_display_get_registry(ptr->display);
  wl_registry_add_listener(wl_registry, &listener, ptr);
  wl_display_roundtrip(ptr->display);
  if (ptr->compositor == NULL || ptr->layer_shell == NULL ||
      ptr->output == NULL) {
    LOG("No compositor\n");
    goto error;
  }

  ptr->surface = wl_compositor_create_surface(ptr->compositor);
  if (ptr->surface == NULL) {
    LOG("No Compositor surface\n");
    goto error;
  }

  ptr->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      ptr->layer_shell, ptr->surface, ptr->output,
      ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wayab");

  zwlr_layer_surface_v1_set_size(ptr->layer_surface, 0, 0);
  zwlr_layer_surface_v1_set_anchor(ptr->layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
  zwlr_layer_surface_v1_set_exclusive_zone(ptr->layer_surface, -1);
  zwlr_layer_surface_v1_add_listener(ptr->layer_surface,
                                     &layer_surface_listener, ptr);
  wl_surface_commit(ptr->surface);

  wl_display_roundtrip(ptr->display);

  return ptr;
error:
  if (ptr)
    wayab_wl_destroy(ptr);
  return NULL;
}

int wayab_wl_destroy(struct wayab_wl *ptr) {
  zwlr_layer_surface_v1_destroy(ptr->layer_surface);
  wl_surface_destroy(ptr->surface);
  wl_display_disconnect(ptr->display);
  free(ptr);
  return 0;
}
