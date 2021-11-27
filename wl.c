#include "wl.h"
#include "render.h"

#include <sys/epoll.h>

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
  } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
    ptr->output_manager =
        wl_registry_bind(registry, id, &zxdg_output_manager_v1_interface, 2);
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
  if (ptr->compositor == NULL || ptr->layer_shell == NULL ||
      ptr->output_manager == NULL) {
    LOG("wl_registry_add_listener\n");
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
  zxdg_output_manager_v1_destroy(ptr->output_manager);
  wl_display_disconnect(ptr->display);
  free(ptr);
  return 0;
}

#define MAX_EVENTS 16

void wayab_wl_loop(struct wayab_wl *wl) {
  struct epoll_event ev, events[MAX_EVENTS];

  int wl_fd = wl_display_get_fd(wl->display);

  int epollfd = epoll_create1(0);
  if (epollfd == -1) {
    perror("epoll_create1");
    exit(EXIT_FAILURE);
  }

  ev.events = EPOLLIN;
  ev.data.fd = wl_fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, wl_fd, &ev) == -1) {
    perror("epoll_ctl");
    exit(EXIT_FAILURE);
  }

  while (1) {
    wl_display_dispatch_pending(wl->display);
    struct wayab_renderer *renderer, *tmp;
    wl_list_for_each_safe(renderer, tmp, &wl->renderers, link) {
      wayab_renderer_draw(renderer);
    }
    int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
  }
}
