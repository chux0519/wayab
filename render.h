#ifndef _WAYAB_RENDER_H
#define _WAYAB_RENDER_H

#include "image.h"
#include "wl.h"

#include <wayland-egl.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#include <cairo.h>

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

  char *name; // by xdg output manager
  int width;  // by layer_surface listener
  int height;

  cairo_surface_t *cairo_surface;
  cairo_device_t *cairo_device;
  cairo_t *cr;

  struct wayab_image *image;

  struct wl_list link;
};

struct wayab_renderer *wayab_renderer_new(struct wl_output *,
                                          struct wayab_wl *);
int wayab_renderer_destroy(struct wayab_renderer *);
int wayab_renderer_draw(struct wayab_renderer *, uint64_t counter);

#endif
