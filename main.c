#include "render.h"
#include "wl.h"

int main() {

  struct wayab_wl *wl = wayab_wl_new();
  if (wl == NULL) {
    LOG("wayab_wl_new");
    return -1;
  }

  while (1) {
    struct wayab_renderer *renderer, *tmp;
    wl_list_for_each_safe(renderer, tmp, &wl->renderers, link) {
      wl_display_dispatch_pending(renderer->native_display);
      wayab_renderer_draw(renderer);
    }
  }

  wayab_wl_destroy(wl);

  return 0;
}
