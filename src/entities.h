#ifndef ENTITIES_H
#define ENTITIES_H

#include <stdint.h>

#define MAX_BULLETS 512
#define MAX_ENEMIES 192
#define MAX_PICKUPS 64

typedef struct {
  float x;
  float y;
  float vx;
  float vy;
  float speed;
  float radius;
  float life;
  int damage;
  uint8_t alive;
  uint8_t homing;
} Bullet;

typedef struct {
  float x;
  float y;
  float radius;
  float hp;
  float speed;
  uint8_t alive;
} Enemy;

typedef struct {
  float x;
  float y;
  float radius;
  uint8_t alive;
  uint8_t powerupType;
  uint8_t isActive;
} Pickup;

void bullets_clear(Bullet bullets[MAX_BULLETS]);
void enemies_clear(Enemy enemies[MAX_ENEMIES]);
void pickups_clear(Pickup pickups[MAX_PICKUPS]);

int bullet_spawn(Bullet bullets[MAX_BULLETS], float x, float y, float vx, float vy,
                 float speed, float radius, float life, int damage, int homing);
int enemy_spawn(Enemy enemies[MAX_ENEMIES], float x, float y, float radius, float hp, float speed);
int pickup_spawn(Pickup pickups[MAX_PICKUPS], float x, float y, float radius,
                 uint8_t powerupType, uint8_t isActive);

int find_nearest_enemy(const Enemy enemies[MAX_ENEMIES], float x, float y, float* outDistSq);
float dist_sq(float x1, float y1, float x2, float y2);

#endif
