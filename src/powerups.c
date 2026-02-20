#include "powerups.h"

void powerup_init_stats(PlayerStats* stats) {
  stats->moveSpeed = 130.0f;
  stats->fireInterval = 0.17f;
  stats->multiShot = 1;
  stats->hasActive = 0;
  stats->activeType = POWERUP_ACTIVE_HOMING_MISSILE;
}

void powerup_apply_passive(PlayerStats* stats, PowerupType type) {
  switch (type) {
    case POWERUP_PASSIVE_MOVE_SPEED:
      stats->moveSpeed += 12.0f;
      if (stats->moveSpeed > 320.0f) {
        stats->moveSpeed = 320.0f;
      }
      break;

    case POWERUP_PASSIVE_FIRE_RATE:
      stats->fireInterval -= 0.018f;
      if (stats->fireInterval < 0.05f) {
        stats->fireInterval = 0.05f;
      }
      break;

    case POWERUP_PASSIVE_MULTI_SHOT:
      stats->multiShot += 1;
      if (stats->multiShot > 6) {
        stats->multiShot = 6;
      }
      break;

    default:
      break;
  }
}

const char* powerup_name(PowerupType type) {
  switch (type) {
    case POWERUP_PASSIVE_MOVE_SPEED:
      return "Move +";
    case POWERUP_PASSIVE_FIRE_RATE:
      return "Fire +";
    case POWERUP_PASSIVE_MULTI_SHOT:
      return "Spread +";
    case POWERUP_ACTIVE_HOMING_MISSILE:
      return "Homing Missile";
    default:
      return "Unknown";
  }
}

int powerup_is_active(PowerupType type) {
  return (int)type >= 100;
}
