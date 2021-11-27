#include "render.h"
#include "wl.h"

int main() {

  struct wayab_wl *wl = wayab_wl_new();
  if (wl == NULL) {
    LOG("wayab_wl_new");
    return -1;
  }

  while (1) {
    wl_display_dispatch_pending(wl->display);
    struct wayab_renderer *renderer, *tmp;
    wl_list_for_each_safe(renderer, tmp, &wl->renderers, link) {
      wayab_renderer_draw(renderer);
    }
  }

  wayab_wl_destroy(wl);

  return 0;
}
