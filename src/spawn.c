#include "spawn.h"

#include "entities.h"

#include <stdlib.h>

int spawn_point_in_arena(int arenaSize, float minDistanceFromPlayer, float playerX, float playerY,
                         float* outX, float* outY) {
  const float minDistSq = minDistanceFromPlayer * minDistanceFromPlayer;

  for (int i = 0; i < 64; i++) {
    const float x = (float)(rand() % arenaSize);
    const float y = (float)(rand() % arenaSize);
    if (dist_sq(x, y, playerX, playerY) >= minDistSq) {
      *outX = x;
      *outY = y;
      return 1;
    }
  }

  *outX = (playerX + minDistanceFromPlayer + 32.0f);
  *outY = playerY;
  if (*outX >= (float)arenaSize) {
    *outX = (float)arenaSize - 1.0f;
  }
  if (*outY >= (float)arenaSize) {
    *outY = (float)arenaSize - 1.0f;
  }
  if (*outX < 0.0f) {
    *outX = 0.0f;
  }
  if (*outY < 0.0f) {
    *outY = 0.0f;
  }
  return 0;
}
