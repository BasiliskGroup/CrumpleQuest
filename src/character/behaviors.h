#ifndef BEHAVIORS_H
#define BEHAVIORS_H

#include "character/behavior.h"

// Chase player behavior - follows the player using pathfinding
class ChasePlayerBehavior : public Behavior {
public:
    void update(Enemy* enemy, const vec2& playerPos, float dt) override;
    std::string getName() const override { return "ChasePlayer"; }
};

// Runaway behavior - runs away from the player
class RunawayBehavior : public Behavior {
public:
    void update(Enemy* enemy, const vec2& playerPos, float dt) override;
    std::string getName() const override { return "Runaway"; }
    
private:
    float minDistance = 5.0f;  // Minimum distance to maintain from player
    float panicDistance = 2.0f;  // Distance at which to start running
};

// Idle behavior - stands still and does nothing
class IdleBehavior : public Behavior {
public:
    void update(Enemy* enemy, const vec2& playerPos, float dt) override;
    std::string getName() const override { return "Idle"; }
};

// Stationary behavior - stands still but attacks if player is in range
class StationaryBehavior : public Behavior {
public:
    void update(Enemy* enemy, const vec2& playerPos, float dt) override;
    std::string getName() const override { return "Stationary"; }
    
private:
    float attackRange = 3.0f;  // Range at which to attack
};

// Wander behavior - moves to random non-obstacle positions
class WanderBehavior : public Behavior {
public:
    void update(Enemy* enemy, const vec2& playerPos, float dt) override;
    std::string getName() const override { return "Wander"; }
};

#endif

