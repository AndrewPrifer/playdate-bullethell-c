#include "entities.h"

#include <math.h>
#include <stddef.h>

float dist_sq(float x1, float y1, float x2, float y2) {
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  return dx * dx + dy * dy;
}

void bullets_clear(Bullet bullets[MAX_BULLETS]) {
  for (size_t i = 0; i < MAX_BULLETS; i++) {
    bullets[i].alive = 0;
  }
}

void enemies_clear(Enemy enemies[MAX_ENEMIES]) {
  for (size_t i = 0; i < MAX_ENEMIES; i++) {
    enemies[i].alive = 0;
  }
}

void pickups_clear(Pickup pickups[MAX_PICKUPS]) {
  for (size_t i = 0; i < MAX_PICKUPS; i++) {
    pickups[i].alive = 0;
  }
}

int bullet_spawn(Bullet bullets[MAX_BULLETS], float x, float y, float vx, float vy,
                 float speed, float radius, float life, int damage, int homing) {
  for (size_t i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].alive) {
      bullets[i].alive = 1;
      bullets[i].x = x;
      bullets[i].y = y;
      bullets[i].vx = vx;
      bullets[i].vy = vy;
      bullets[i].speed = speed;
      bullets[i].radius = radius;
      bullets[i].life = life;
      bullets[i].damage = damage;
      bullets[i].homing = homing ? 1 : 0;
      return 1;
    }
  }

  return 0;
}

int enemy_spawn(Enemy enemies[MAX_ENEMIES], float x, float y, float radius, float hp, float speed) {
  for (size_t i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) {
      enemies[i].alive = 1;
      enemies[i].x = x;
      enemies[i].y = y;
      enemies[i].radius = radius;
      enemies[i].hp = hp;
      enemies[i].speed = speed;
      return 1;
    }
  }

  return 0;
}

int pickup_spawn(Pickup pickups[MAX_PICKUPS], float x, float y, float radius,
                 uint8_t powerupType, uint8_t isActive) {
  for (size_t i = 0; i < MAX_PICKUPS; i++) {
    if (!pickups[i].alive) {
      pickups[i].alive = 1;
      pickups[i].x = x;
      pickups[i].y = y;
      pickups[i].radius = radius;
      pickups[i].powerupType = powerupType;
      pickups[i].isActive = isActive;
      return 1;
    }
  }

  return 0;
}

int find_nearest_enemy(const Enemy enemies[MAX_ENEMIES], float x, float y, float* outDistSq) {
  int idx = -1;
  float best = 0.0f;
  for (size_t i = 0; i < MAX_ENEMIES; i++) {
    if (!enemies[i].alive) {
      continue;
    }

    const float d2 = dist_sq(x, y, enemies[i].x, enemies[i].y);
    if (idx < 0 || d2 < best) {
      idx = (int)i;
      best = d2;
    }
  }

  if (outDistSq != NULL && idx >= 0) {
    *outDistSq = best;
  }

  return idx;
}
