#include <wayland-client.h>
#include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers

#include "protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "protocols/xdg-shell-client-protocol.h"

#include <stdbool.h>
#include <stdio.h>
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERRNO(...)                                                         \
  fprintf(stderr, "Error : %s\n", strerror(errno));                            \
  fprintf(stderr, __VA_ARGS__)

#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglplatform.h>

struct _escontext {
  /// Native System informations
  EGLNativeDisplayType native_display;
  EGLNativeWindowType native_window;
  uint16_t window_width, window_height;
  /// EGL display
  EGLDisplay display;
  /// EGL context
  EGLContext context;
  /// EGL surface
  EGLSurface surface;
};

void CreateNativeWindow(char *title, int width, int height);
EGLBoolean CreateEGLContext();
EGLBoolean CreateWindowWithEGLContext(char *title, int width, int height);
void RefreshWindow();

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include <sys/time.h>

#include <GLES2/gl2.h>

struct wl_display *display;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface;
struct wl_output *output;
struct wl_egl_window *egl_window;
struct wl_region *region;

struct xdg_wm_base *XDGWMBase;
struct xdg_surface *XDGSurface;
struct xdg_toplevel *XDGToplevel;

struct zwlr_layer_shell_v1 *layer_shell;
struct zwlr_layer_surface_v1 *layer_surface;

struct _escontext ESContext = {.native_display = NULL,
                               .window_width = 0,
                               .window_height = 0,
                               .native_window = 0,
                               .display = NULL,
                               .context = NULL,
                               .surface = NULL};

#define TRUE 1
#define FALSE 0

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height) {
  // The entire surface can be marked opaque, as it should be the lowest
  // z-index on the display.
  struct wl_region *opaque = wl_compositor_create_region(compositor);
  wl_region_add(opaque, 0, 0, width, height);
  wl_surface_set_opaque_region(surface, opaque);
  wl_region_destroy(opaque);

  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface
                                 __attribute__((unused))) {}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

bool program_alive;
int32_t old_w, old_h;

static void xdg_toplevel_handle_configure(void *data,
                                          struct xdg_toplevel *xdg_toplevel,
                                          int32_t w, int32_t h,
                                          struct wl_array *states) {

  // no window geometry event, ignore
  if (w == 0 && h == 0)
    return;

  // window resized
  if (old_w != w && old_h != h) {
    old_w = w;
    old_h = h;

    wl_egl_window_resize(ESContext.native_window, w, h, 0, 0);
    wl_surface_commit(surface);
  }
}

static void xdg_toplevel_handle_close(void *data,
                                      struct xdg_toplevel *xdg_toplevel) {
  // window closed, be sure that this event gets processed
  program_alive = false;
}

struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial) {
  // confirm that you exist to the compositor
  xdg_surface_ack_configure(xdg_surface, serial);
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

void CreateNativeWindow(char *title, int width, int height) {
  old_w = WINDOW_WIDTH;
  old_h = WINDOW_HEIGHT;

  // region = wl_compositor_create_region(compositor);

  // wl_region_add(region, 0, 0, width, height);
  // wl_surface_set_opaque_region(surface, region);

  // 通过 zwlr_layer_shell_v1::get_layer_surface 给 surface 添加 role

  struct wl_egl_window *egl_window =
      wl_egl_window_create(surface, width, height);

  if (egl_window == EGL_NO_SURFACE) {
    LOG("No window !?\n");
    exit(1);
  } else
    LOG("Window created !\n");
  ESContext.window_width = width;
  ESContext.window_height = height;
  ESContext.native_window = egl_window;
}

EGLBoolean CreateEGLContext() {
  EGLint numConfigs;
  EGLint majorVersion;
  EGLint minorVersion;
  EGLContext context;
  EGLSurface surface;
  EGLConfig config;
  EGLint fbAttribs[] = {EGL_SURFACE_TYPE,
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
  EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
  EGLDisplay display = eglGetDisplay(ESContext.native_display);
  if (display == EGL_NO_DISPLAY) {
    LOG("No EGL Display...\n");
    return EGL_FALSE;
  }

  // Initialize EGL
  if (!eglInitialize(display, &majorVersion, &minorVersion)) {
    LOG("No Initialisation...\n");
    return EGL_FALSE;
  }

  // Get configs
  if ((eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE) ||
      (numConfigs == 0)) {
    LOG("No configuration...\n");
    return EGL_FALSE;
  }

  // Choose config
  if ((eglChooseConfig(display, fbAttribs, &config, 1, &numConfigs) !=
       EGL_TRUE) ||
      (numConfigs != 1)) {
    LOG("No configuration...\n");
    return EGL_FALSE;
  }

  // Create a surface
  surface =
      eglCreateWindowSurface(display, config, ESContext.native_window, NULL);
  if (surface == EGL_NO_SURFACE) {
    LOG("No surface...\n");
    return EGL_FALSE;
  }

  // Create a GL context
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT) {
    LOG("No context...\n");
    return EGL_FALSE;
  }

  // Make the context current
  if (!eglMakeCurrent(display, surface, surface, context)) {
    LOG("Could not make the current window current !\n");
    return EGL_FALSE;
  }

  ESContext.display = display;
  ESContext.surface = surface;
  ESContext.context = context;
  return EGL_TRUE;
}

EGLBoolean CreateWindowWithEGLContext(char *title, int width, int height) {
  CreateNativeWindow(title, width, height);
  return CreateEGLContext();
}

void draw() {
  glClearColor(0.5, 0.0, 0.0, 1.0);

  // struct timeval tv;

  // gettimeofday(&tv, NULL);

  // float time = tv.tv_sec + tv.tv_usec/1000000.0;

  // static GLfloat vertex_data[] = {
  //	0.6, 0.6, 1.0,
  //	-0.6, -0.6, 1.0,
  //	0.0, 1.0, 1.0
  // };

  // for(int i=0; i<3; i++) {
  //	vertex_data[i*3+0] = vertex_data[i*3+0]*cos(time) -
  // vertex_data[i*3+1]*sin(time); 	vertex_data[i*3+1] =
  // vertex_data[i*3+0]*sin(time) + vertex_data[i*3+1]*cos(time);
  // }

  glClear(GL_COLOR_BUFFER_BIT);

  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertex_data);
  // glEnableVertexAttribArray(0);

  // glDrawArrays(GL_TRIANGLES, 0, 3);
}

unsigned long last_click = 0;
void RefreshWindow() { eglSwapBuffers(ESContext.display, ESContext.surface); }

static void global_registry_handler(void *data, struct wl_registry *registry,
                                    uint32_t id, const char *interface,
                                    uint32_t version) {
  LOG("Got a registry event for %s id %d\n", interface, id);
  if (strcmp(interface, "wl_compositor") == 0)
    compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
  else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    XDGWMBase = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(XDGWMBase, &xdg_wm_base_listener, NULL);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    layer_shell =
        wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wl_output_interface.name) == 0) {
    output = wl_registry_bind(registry, id, &wl_output_interface, 3);
  }
}

static void global_registry_remover(void *data, struct wl_registry *registry,
                                    uint32_t id) {
  LOG("Got a registry losing event for %d\n", id);
}

const struct wl_registry_listener listener = {global_registry_handler,
                                              global_registry_remover};

static void get_server_references() {

  display = wl_display_connect(NULL);
  if (display == NULL) {
    LOG("Can't connect to wayland display !?\n");
    exit(1);
  }
  LOG("Got a display !");

  struct wl_registry *wl_registry = wl_display_get_registry(display);
  wl_registry_add_listener(wl_registry, &listener, NULL);

  // This call the attached listener global_registry_handler
  wl_display_dispatch(display);
  wl_display_roundtrip(display);

  // If at this point, global_registry_handler didn't set the
  // compositor, nor the shell, bailout !
  if (compositor == NULL || XDGWMBase == NULL) {
    LOG("No compositor !? No XDG !! There's NOTHING in here !\n");
    exit(1);
  } else {
    LOG("Okay, we got a compositor and a shell... That's something !\n");
    ESContext.native_display = display;
  }
}

void destroy_window() {
  eglDestroySurface(ESContext.display, ESContext.surface);
  wl_egl_window_destroy(ESContext.native_window);
  // xdg_toplevel_destroy(XDGToplevel);
  zwlr_layer_surface_v1_destroy(layer_surface);
  // xdg_surface_destroy(XDGSurface);
  wl_surface_destroy(surface);
  eglDestroyContext(ESContext.display, ESContext.context);
}

int main() {
  get_server_references();

  surface = wl_compositor_create_surface(compositor);
  if (surface == NULL) {
    LOG("No Compositor surface ! Yay....\n");
    exit(1);
  } else
    LOG("Got a compositor surface !\n");

  // XDGSurface = xdg_wm_base_get_xdg_surface(XDGWMBase, surface);

  // xdg_surface_add_listener(XDGSurface, &xdg_surface_listener, NULL);

  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, surface, output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND,
      "wallpaper");

  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     output);
  wl_surface_commit(surface);

  // Get the rest of the output properties. We need the name to associate
  // this output with an oguri_output_config, and oguri_reconfigure assumes
  // it is already present.
  wl_display_roundtrip(display);

  // XDGToplevel = xdg_surface_get_toplevel(XDGSurface);
  // xdg_toplevel_set_title(XDGToplevel, "Wayland EGL example");
  // xdg_toplevel_add_listener(XDGToplevel, &xdg_toplevel_listener, NULL);

  CreateWindowWithEGLContext("Nya", 1920, 1080);

  program_alive = true;

  while (program_alive) {
    wl_display_dispatch_pending(ESContext.native_display);
    draw();
    RefreshWindow();
  }

  destroy_window();
  wl_display_disconnect(ESContext.native_display);
  LOG("Display disconnected !\n");

  exit(0);
}
