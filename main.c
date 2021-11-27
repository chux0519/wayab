#include "render.h"
#include "wl.h"

int main() {

  struct wayab_wl *wl = wayab_wl_new();
  if (wl == NULL) {
    LOG("wayab_wl_new");
    return -1;
  }

  wayab_wl_loop(wl);

  wayab_wl_destroy(wl);

  return 0;
}
