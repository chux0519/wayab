#include "render.h"
#include "wl.h"

int main() {

  struct wayab_wl *wl = wayab_wl_new();
  if (wl == NULL) {
    LOG("wayab_wl_new");
    return -1;
  }

  struct wayab_renderer *renderer = wayab_renderer_new(wl, 1920, 1080);
  if (renderer == NULL) {
    LOG("wayab_renderer_new");
    return -1;
  }

  while (1) {
    wl_display_dispatch_pending(renderer->native_display);
    wayab_renderer_draw(renderer);
  }

  wayab_renderer_destroy(renderer);
  wayab_wl_destroy(wl);

  return 0;
}
