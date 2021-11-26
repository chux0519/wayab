#ifndef _WAYAB_WL_H
#define _WAYAB_WL_H

#include <stddef.h>
#include <stdio.h>

#include <wayland-client.h>

#include "protocols/wlr-layer-shell-unstable-v1-client-protocol.h"

#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERRNO(...)                                                         \
  fprintf(stderr, "Error : %s\n", strerror(errno));                            \
  fprintf(stderr, __VA_ARGS__)

/// init process:
/// 1. connect display
/// 2. block display listener
///   2.1 compositor
///   2.2 output
///   2.3 layer_shell
/// 3. surface created by compositor
/// 4. layer_shell listener
///   4.1 layer_surface
/// 5. layer_surface listener
///   5.1 region
struct wayab_wl {
  /* connect */
  struct wl_display *display;

  /* display listener */
  struct wl_compositor *compositor;
  struct wl_output *output;
  struct zwlr_layer_shell_v1 *layer_shell;

  /* created by compositor */
  struct wl_surface *surface;

  /* created by layer_shell */
  struct zwlr_layer_surface_v1 *layer_surface;

  /* created by surface listener */
  struct wl_region *region;
};

struct wayab_wl *wayab_wl_new();
int wayab_wl_destroy(struct wayab_wl *);

#endif
