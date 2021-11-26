#include "render.h"

#include <stdlib.h>

#include <GLES2/gl2.h>

struct wayab_renderer *wayab_renderer_new(struct wayab_wl *wl, int width,
                                          int height) {
  struct wayab_renderer *ptr = calloc(1, sizeof(struct wayab_renderer));
  ptr->native_display = wl->display;
  ptr->native_window = wl_egl_window_create(wl->surface, width, height);

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

  if (!eglMakeCurrent(ptr->display, ptr->surface, ptr->surface, ptr->context)) {
    LOG("eglMakeCurrent\n");
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
  eglDestroyContext(ptr->display, ptr->context);
  free(ptr);
  return 0;
}

int wayab_renderer_draw(struct wayab_renderer *ptr) {
  glClearColor(0.5, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  eglSwapBuffers(ptr->display, ptr->surface);
  return 0;
}
