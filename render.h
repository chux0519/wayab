#ifndef _WAYAB_RENDER_H
#define _WAYAB_RENDER_H

#include "wl.h"

#include <wayland-egl.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

struct wayab_renderer {
  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;

  EGLDisplay display;
  EGLContext context;
  EGLSurface surface;

  struct wl_output *wl_output;
  struct wl_surface *wl_surface;
  struct wl_region *wl_region;
  struct zwlr_layer_surface_v1 *layer_surface;

  struct wl_list link;
};

struct wayab_renderer *wayab_renderer_new(struct wl_output *,
                                          struct wayab_wl *);
int wayab_renderer_destroy(struct wayab_renderer *);
int wayab_renderer_draw(struct wayab_renderer *);

#endif
