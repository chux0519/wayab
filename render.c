#include "render.h"

#include <stdlib.h>

#include <GLES2/gl2.h>

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  struct wayab_renderer *ptr = data;

  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface
                                 __attribute__((unused))) {}

static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

struct wayab_renderer *wayab_renderer_new(struct wl_output *wl_output,
                                          struct wayab_wl *wl) {
  // FIXME: read from output
  int width = 1920;
  int height = 1080;

  struct wayab_renderer *ptr = calloc(1, sizeof(struct wayab_renderer));
  ptr->native_display = wl->display;
  ptr->wl_output = wl_output;

  // init surface
  ptr->wl_surface = wl_compositor_create_surface(wl->compositor);
  if (ptr->wl_surface == NULL) {
    LOG("No Compositor surface\n");
    goto error;
  }

  // init layer_surface
  ptr->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      wl->layer_shell, ptr->wl_surface, wl_output,
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
  wl_surface_commit(ptr->wl_surface);
  wl_display_roundtrip(wl->display);

  // init window
  ptr->native_window = wl_egl_window_create(ptr->wl_surface, width, height);

  if (ptr->native_window == EGL_NO_SURFACE) {
    LOG("Failed to create egl window\n");
    goto error;
  }

  ptr->display = eglGetDisplay(ptr->native_display);
  if (ptr->display == EGL_NO_DISPLAY) {
    goto error;
  }

  EGLint major_version;
  EGLint minor_version;
  if (!eglInitialize(ptr->display, &major_version, &minor_version)) {
    LOG("eglInitialize\n");
    goto error;
  }

  EGLint num_configs;
  if ((eglGetConfigs(ptr->display, NULL, 0, &num_configs) != EGL_TRUE) ||
      (num_configs == 0)) {
    LOG("eglGetConfigs\n");
    goto error;
  }

  EGLint attrib_list[] = {EGL_SURFACE_TYPE,
                          EGL_WINDOW_BIT,
                          EGL_RENDERABLE_TYPE,
                          EGL_OPENGL_ES2_BIT,
                          EGL_RED_SIZE,
                          8,
                          EGL_GREEN_SIZE,
                          8,
                          EGL_BLUE_SIZE,
                          8,
                          EGL_NONE};
  EGLConfig config;
  if ((eglChooseConfig(ptr->display, attrib_list, &config, 1, &num_configs) !=
       EGL_TRUE) ||
      (num_configs != 1)) {
    LOG("eglChooseConfig\n");
    goto error;
  }

  ptr->surface =
      eglCreateWindowSurface(ptr->display, config, ptr->native_window, NULL);
  if (ptr->surface == EGL_NO_SURFACE) {
    LOG("eglCreateWindowSurface\n");
    goto error;
  }

  EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
                              EGL_NONE};
  ptr->context =
      eglCreateContext(ptr->display, config, EGL_NO_CONTEXT, context_attribs);
  if (ptr->context == EGL_NO_CONTEXT) {
    LOG("eglCreateContext\n");
    goto error;
  }

  return ptr;

error:
  if (ptr)
    wayab_renderer_destroy(ptr);
  return NULL;
}

int wayab_renderer_destroy(struct wayab_renderer *ptr) {
  eglDestroySurface(ptr->display, ptr->surface);
  wl_egl_window_destroy(ptr->native_window);
  zwlr_layer_surface_v1_destroy(ptr->layer_surface);
  wl_surface_destroy(ptr->wl_surface);
  eglDestroyContext(ptr->display, ptr->context);
  free(ptr);

  return 0;
}

int wayab_renderer_draw(struct wayab_renderer *ptr) {
  if (!eglMakeCurrent(ptr->display, ptr->surface, ptr->surface, ptr->context)) {
    LOG("eglMakeCurrent\n");
    return -1;
  }
  glClearColor(0.5, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  eglSwapBuffers(ptr->display, ptr->surface);
  return 0;
}
