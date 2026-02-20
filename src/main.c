#include "pd_api.h"

#include "game.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
  (void)arg;

  if (event == kEventInit) {
    game_init(playdate);
    playdate->display->setRefreshRate(30.0f);
    playdate->system->setUpdateCallback(game_update, NULL);
  }

  return 0;
}
