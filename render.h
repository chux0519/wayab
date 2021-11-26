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
};

struct wayab_renderer *wayab_renderer_new(struct wayab_wl *, int, int);
int wayab_renderer_destroy(struct wayab_renderer *);
int wayab_renderer_draw(struct wayab_renderer *);

#endif
