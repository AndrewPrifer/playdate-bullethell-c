#ifndef POWERUPS_H
#define POWERUPS_H

#include <stdint.h>

typedef enum {
  POWERUP_PASSIVE_MOVE_SPEED = 0,
  POWERUP_PASSIVE_FIRE_RATE = 1,
  POWERUP_PASSIVE_MULTI_SHOT = 2,
  POWERUP_ACTIVE_HOMING_MISSILE = 100,
} PowerupType;

typedef struct {
  float moveSpeed;
  float fireInterval;
  int multiShot;
  int hasActive;
  PowerupType activeType;
} PlayerStats;

void powerup_init_stats(PlayerStats* stats);
void powerup_apply_passive(PlayerStats* stats, PowerupType type);
const char* powerup_name(PowerupType type);
int powerup_is_active(PowerupType type);

#endif
