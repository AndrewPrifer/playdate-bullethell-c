#include "game.h"

#include "entities.h"
#include "powerups.h"
#include "spawn.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  float x;
  float y;
  float radius;
  int hp;
} Player;

typedef struct {
  Player player;
  PlayerStats stats;
  Bullet bullets[MAX_BULLETS];
  Enemy enemies[MAX_ENEMIES];
  Pickup pickups[MAX_PICKUPS];
  float camX;
  float camY;
  float fireTimer;
  float enemySpawnTimer;
  float powerupSpawnTimer;
  float waveTimer;
  int wave;
  int score;
  int gameOver;
  uint32_t lastTickMs;
} GameState;

static PlaydateAPI* pd = NULL;
static GameState g;

enum {
  SCREEN_W = 400,
  SCREEN_H = 240,
  ARENA_SIZE = 2000,
  GRID_STEP = 80,
};

static float clampf(float value, float minv, float maxv) {
  if (value < minv) {
    return minv;
  }
  if (value > maxv) {
    return maxv;
  }
  return value;
}

static float deg_to_rad(float deg) {
  return deg * 3.14159265f / 180.0f;
}

static void reset_round(void) {
  powerup_init_stats(&g.stats);
  bullets_clear(g.bullets);
  enemies_clear(g.enemies);
  pickups_clear(g.pickups);
  g.player.x = ARENA_SIZE * 0.5f;
  g.player.y = ARENA_SIZE * 0.5f;
  g.player.radius = 7.0f;
  g.player.hp = 8;
  g.wave = 1;
  g.score = 0;
  g.fireTimer = 0.0f;
  g.enemySpawnTimer = 0.3f;
  g.powerupSpawnTimer = 6.0f;
  g.waveTimer = 18.0f;
  g.camX = 0.0f;
  g.camY = 0.0f;
  g.gameOver = 0;
}

static void spawn_enemy(void) {
  float x = 0.0f;
  float y = 0.0f;
  (void)spawn_point_in_arena(ARENA_SIZE, 170.0f, g.player.x, g.player.y, &x, &y);

  const float hp = 1.0f + (float)(g.wave - 1) * 0.25f;
  const float speed = 42.0f + (float)(g.wave - 1) * 2.5f;
  (void)enemy_spawn(g.enemies, x, y, 8.0f, hp, speed);
}

static void spawn_powerup(void) {
  float x = 0.0f;
  float y = 0.0f;
  (void)spawn_point_in_arena(ARENA_SIZE, 190.0f, g.player.x, g.player.y, &x, &y);

  const int activeRoll = rand() % 5 == 0;
  PowerupType type = POWERUP_PASSIVE_MOVE_SPEED;
  uint8_t isActive = 0;

  if (activeRoll) {
    type = POWERUP_ACTIVE_HOMING_MISSILE;
    isActive = 1;
  } else {
    const int roll = rand() % 3;
    if (roll == 0) {
      type = POWERUP_PASSIVE_MOVE_SPEED;
    } else if (roll == 1) {
      type = POWERUP_PASSIVE_FIRE_RATE;
    } else {
      type = POWERUP_PASSIVE_MULTI_SHOT;
    }
  }

  (void)pickup_spawn(g.pickups, x, y, 6.0f, (uint8_t)type, isActive);
}

static void fire_player_gun(float crankDeg) {
  g.fireTimer -= g.stats.fireInterval;

  const int shots = g.stats.multiShot;
  const float spreadStep = 7.0f;
  const float start = -(float)(shots - 1) * 0.5f * spreadStep;

  for (int i = 0; i < shots; i++) {
    const float deg = crankDeg + start + (float)i * spreadStep;
    const float rad = deg_to_rad(deg);
    const float vx = sinf(rad);
    const float vy = -cosf(rad);
    (void)bullet_spawn(g.bullets, g.player.x, g.player.y, vx, vy, 290.0f, 2.0f, 1.8f, 1, 0);
  }
}

static void trigger_active_power(void) {
  if (!g.stats.hasActive || g.stats.activeType != POWERUP_ACTIVE_HOMING_MISSILE) {
    return;
  }

  for (int i = 0; i < 6; i++) {
    const float angle = (float)(i * 60);
    const float rad = deg_to_rad(angle);
    (void)bullet_spawn(g.bullets, g.player.x, g.player.y, sinf(rad), -cosf(rad),
                       210.0f, 3.0f, 2.5f, 3, 1);
  }

  g.stats.hasActive = 0;
}

static void update_camera(void) {
  g.camX = g.player.x - ((float)SCREEN_W * 0.5f);
  g.camY = g.player.y - ((float)SCREEN_H * 0.5f);
  g.camX = clampf(g.camX, 0.0f, (float)(ARENA_SIZE - SCREEN_W));
  g.camY = clampf(g.camY, 0.0f, (float)(ARENA_SIZE - SCREEN_H));
}

static void update_player(float dt, PDButtons current, PDButtons pushed) {
  float moveX = 0.0f;
  float moveY = 0.0f;

  if (current & kButtonLeft) {
    moveX -= 1.0f;
  }
  if (current & kButtonRight) {
    moveX += 1.0f;
  }
  if (current & kButtonUp) {
    moveY -= 1.0f;
  }
  if (current & kButtonDown) {
    moveY += 1.0f;
  }

  if (moveX != 0.0f && moveY != 0.0f) {
    moveX *= 0.70710677f;
    moveY *= 0.70710677f;
  }

  g.player.x += moveX * g.stats.moveSpeed * dt;
  g.player.y += moveY * g.stats.moveSpeed * dt;
  g.player.x = clampf(g.player.x, g.player.radius, (float)ARENA_SIZE - g.player.radius);
  g.player.y = clampf(g.player.y, g.player.radius, (float)ARENA_SIZE - g.player.radius);

  if (pushed & kButtonA) {
    trigger_active_power();
  }
}

static void update_enemies(float dt) {
  for (int i = 0; i < MAX_ENEMIES; i++) {
    Enemy* e = &g.enemies[i];
    if (!e->alive) {
      continue;
    }

    const float dx = g.player.x - e->x;
    const float dy = g.player.y - e->y;
    const float lenSq = dx * dx + dy * dy;
    if (lenSq > 0.0001f) {
      const float invLen = 1.0f / sqrtf(lenSq);
      e->x += dx * invLen * e->speed * dt;
      e->y += dy * invLen * e->speed * dt;
    }
  }
}

static void update_bullets(float dt) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    Bullet* b = &g.bullets[i];
    if (!b->alive) {
      continue;
    }

    if (b->homing) {
      const int idx = find_nearest_enemy(g.enemies, b->x, b->y, NULL);
      if (idx >= 0) {
        const Enemy* e = &g.enemies[idx];
        const float dx = e->x - b->x;
        const float dy = e->y - b->y;
        const float lenSq = dx * dx + dy * dy;
        if (lenSq > 0.0001f) {
          const float invLen = 1.0f / sqrtf(lenSq);
          const float desiredX = dx * invLen;
          const float desiredY = dy * invLen;
          b->vx = b->vx * 0.86f + desiredX * 0.14f;
          b->vy = b->vy * 0.86f + desiredY * 0.14f;
          const float dlenSq = b->vx * b->vx + b->vy * b->vy;
          if (dlenSq > 0.0001f) {
            const float dinv = 1.0f / sqrtf(dlenSq);
            b->vx *= dinv;
            b->vy *= dinv;
          }
        }
      }
    }

    b->x += b->vx * b->speed * dt;
    b->y += b->vy * b->speed * dt;
    b->life -= dt;

    if (b->life <= 0.0f || b->x < 0.0f || b->y < 0.0f || b->x > (float)ARENA_SIZE ||
        b->y > (float)ARENA_SIZE) {
      b->alive = 0;
    }
  }
}

static void handle_collisions(void) {
  for (int i = 0; i < MAX_BULLETS; i++) {
    Bullet* b = &g.bullets[i];
    if (!b->alive) {
      continue;
    }

    for (int j = 0; j < MAX_ENEMIES; j++) {
      Enemy* e = &g.enemies[j];
      if (!e->alive) {
        continue;
      }

      const float rr = b->radius + e->radius;
      if (dist_sq(b->x, b->y, e->x, e->y) <= rr * rr) {
        e->hp -= (float)b->damage;
        b->alive = 0;
        if (e->hp <= 0.0f) {
          e->alive = 0;
          g.score += 10;
        }
        break;
      }
    }
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    Enemy* e = &g.enemies[i];
    if (!e->alive) {
      continue;
    }

    const float rr = g.player.radius + e->radius;
    if (dist_sq(g.player.x, g.player.y, e->x, e->y) <= rr * rr) {
      e->alive = 0;
      g.player.hp -= 1;
      if (g.player.hp <= 0) {
        g.gameOver = 1;
      }
    }
  }

  for (int i = 0; i < MAX_PICKUPS; i++) {
    Pickup* p = &g.pickups[i];
    if (!p->alive) {
      continue;
    }

    const float rr = g.player.radius + p->radius;
    if (dist_sq(g.player.x, g.player.y, p->x, p->y) > rr * rr) {
      continue;
    }

    const PowerupType type = (PowerupType)p->powerupType;
    if (p->isActive) {
      if (!g.stats.hasActive) {
        g.stats.hasActive = 1;
        g.stats.activeType = type;
        p->alive = 0;
      }
    } else {
      powerup_apply_passive(&g.stats, type);
      p->alive = 0;
    }
  }
}

static void update_spawns(float dt) {
  g.waveTimer -= dt;
  if (g.waveTimer <= 0.0f) {
    g.wave += 1;
    g.waveTimer += 18.0f;

    const int burstCount = 2 + g.wave;
    for (int i = 0; i < burstCount; i++) {
      spawn_enemy();
    }
  }

  g.enemySpawnTimer -= dt;
  if (g.enemySpawnTimer <= 0.0f) {
    spawn_enemy();
    const float base = 1.0f - (float)(g.wave - 1) * 0.055f;
    g.enemySpawnTimer += clampf(base, 0.22f, 1.0f);
  }

  g.powerupSpawnTimer -= dt;
  if (g.powerupSpawnTimer <= 0.0f) {
    spawn_powerup();
    g.powerupSpawnTimer += 7.5f;
  }
}

static void draw_world(void) {
  for (int x = GRID_STEP; x < ARENA_SIZE; x += GRID_STEP) {
    pd->graphics->drawLine(x, 0, x, ARENA_SIZE, 1, kColorBlack);
  }
  for (int y = GRID_STEP; y < ARENA_SIZE; y += GRID_STEP) {
    pd->graphics->drawLine(0, y, ARENA_SIZE, y, 1, kColorBlack);
  }
  pd->graphics->drawRect(0, 0, ARENA_SIZE, ARENA_SIZE, kColorBlack);

  pd->graphics->fillRect((int)(g.player.x - 4.0f), (int)(g.player.y - 4.0f), 8, 8, kColorBlack);

  for (int i = 0; i < MAX_BULLETS; i++) {
    const Bullet* b = &g.bullets[i];
    if (!b->alive) {
      continue;
    }
    const int r = (int)b->radius;
    pd->graphics->fillRect((int)b->x - r, (int)b->y - r, r * 2, r * 2, kColorBlack);
  }

  for (int i = 0; i < MAX_ENEMIES; i++) {
    const Enemy* e = &g.enemies[i];
    if (!e->alive) {
      continue;
    }
    const float dx = g.player.x - e->x;
    const float dy = g.player.y - e->y;
    float fx = 0.0f;
    float fy = -1.0f;
    const float lenSq = dx * dx + dy * dy;
    if (lenSq > 0.0001f) {
      const float invLen = 1.0f / sqrtf(lenSq);
      fx = dx * invLen;
      fy = dy * invLen;
    }

    const float px = -fy;
    const float py = fx;
    const float tipX = e->x + fx * e->radius * 1.45f;
    const float tipY = e->y + fy * e->radius * 1.45f;
    const float baseX = e->x - fx * e->radius * 0.9f;
    const float baseY = e->y - fy * e->radius * 0.9f;
    const float leftX = baseX + px * e->radius * 0.95f;
    const float leftY = baseY + py * e->radius * 0.95f;
    const float rightX = baseX - px * e->radius * 0.95f;
    const float rightY = baseY - py * e->radius * 0.95f;

    pd->graphics->drawLine((int)tipX, (int)tipY, (int)leftX, (int)leftY, 1, kColorBlack);
    pd->graphics->drawLine((int)leftX, (int)leftY, (int)rightX, (int)rightY, 1, kColorBlack);
    pd->graphics->drawLine((int)rightX, (int)rightY, (int)tipX, (int)tipY, 1, kColorBlack);
  }

  for (int i = 0; i < MAX_PICKUPS; i++) {
    const Pickup* p = &g.pickups[i];
    if (!p->alive) {
      continue;
    }
    const int r = (int)p->radius;
    if (p->isActive) {
      pd->graphics->drawRect((int)p->x - r, (int)p->y - r, r * 2, r * 2, kColorBlack);
      pd->graphics->drawLine((int)p->x - r, (int)p->y + r, (int)p->x + r, (int)p->y - r, 1, kColorBlack);
    } else {
      pd->graphics->fillRect((int)p->x - r, (int)p->y - r, r * 2, r * 2, kColorBlack);
    }
  }
}

static void draw_hud(void) {
  char line1[96];
  char line2[128];

  (void)snprintf(line1, sizeof(line1), "HP:%d  Wave:%d  Score:%d", g.player.hp, g.wave, g.score);
  pd->graphics->drawText(line1, strlen(line1), kASCIIEncoding, 6, 6);

  if (g.stats.hasActive) {
    (void)snprintf(line2, sizeof(line2), "Tool: %s (A to use)", powerup_name(g.stats.activeType));
  } else {
    (void)snprintf(line2, sizeof(line2), "Tool: Empty");
  }
  pd->graphics->drawText(line2, strlen(line2), kASCIIEncoding, 6, 22);

  if (g.gameOver) {
    const char* msg = "Game Over - Press B to restart";
    pd->graphics->drawText(msg, strlen(msg), kASCIIEncoding, 90, 112);
  }
}

void game_init(PlaydateAPI* playdate) {
  pd = playdate;

  unsigned int seedMs = 0;
  const unsigned int seedSeconds = pd->system->getSecondsSinceEpoch(&seedMs);
  srand(seedSeconds ^ (seedMs << 12));

  reset_round();
  g.lastTickMs = pd->system->getCurrentTimeMilliseconds();
}

int game_update(void* userdata) {
  (void)userdata;

  const uint32_t nowMs = pd->system->getCurrentTimeMilliseconds();
  float dt = (float)(nowMs - g.lastTickMs) / 1000.0f;
  if (dt <= 0.0f || dt > 0.1f) {
    dt = 1.0f / 30.0f;
  }
  g.lastTickMs = nowMs;

  PDButtons current = 0;
  PDButtons pushed = 0;
  PDButtons released = 0;
  pd->system->getButtonState(&current, &pushed, &released);
  (void)released;

  if (g.gameOver) {
    if (pushed & kButtonB) {
      reset_round();
    }
  } else {
    const float crank = pd->system->getCrankAngle();

    update_player(dt, current, pushed);
    g.fireTimer += dt;
    while (g.fireTimer >= g.stats.fireInterval) {
      fire_player_gun(crank);
    }
    update_enemies(dt);
    update_bullets(dt);
    update_spawns(dt);
    handle_collisions();
    update_camera();
  }

  pd->graphics->clear(kColorWhite);
  pd->graphics->setDrawOffset(-(int)g.camX, -(int)g.camY);
  draw_world();
  pd->graphics->setDrawOffset(0, 0);
  draw_hud();

  return 1;
}
