#ifndef _WAYAB_WL_H
#define _WAYAB_WL_H

#include "utils.h"

#include <stddef.h>

#include <wayland-client.h>

#include "protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "protocols/xdg-output-unstable-v1-client-protocol.h"

struct wayab_rule {
  char *output_name;
  char *dir;

  struct wl_list link;
};

struct wayab_config {
  int fps;

  struct wl_list rules;
};

/// init process:
/// 1. connect display
/// 2. block display listener
///   2.1 compositor
///   2.2 output
///   2.3 layer_shell
/// 3. surface created by compositor
/// 4. layer_shell listener
///   4.1 layer_surface with output
/// 5. layer_surface listener
struct wayab_wl {
  /* connect */
  struct wl_display *display;
  /* display listener */
  struct wl_compositor *compositor;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct zxdg_output_manager_v1 *output_manager;

  struct wayab_config *config;
  struct wl_list renderers;
};

struct wayab_wl *wayab_wl_new(struct wayab_config *);
int wayab_wl_destroy(struct wayab_wl *);
void wayab_wl_loop(struct wayab_wl *);

#endif
