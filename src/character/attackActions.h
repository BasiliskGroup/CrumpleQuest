#ifndef ATTACK_ACTIONS_H
#define ATTACK_ACTIONS_H

#include "character/attackAction.h"
#include <unordered_map>

// Normal attack action - single attack
class NormalAttackAction : public AttackAction {
public:
    bool execute(Enemy* enemy, const vec2& pos, const vec2& dir) override;
    std::string getName() const override { return "Normal"; }
};

// Burst attack action - fires multiple projectiles in quick succession
class BurstAttackAction : public AttackAction {
public:
    BurstAttackAction(int numShots = 5, float delayBetweenShots = 0.1f);
    bool execute(Enemy* enemy, const vec2& pos, const vec2& dir) override;
    void update(Enemy* enemy, float dt) override;
    std::string getName() const override { return "Burst"; }
    
private:
    int numShots;
    float delayBetweenShots;
};

// Slide attack action - dashes forward then attacks after delay
class SlideAttackAction : public AttackAction {
public:
    SlideAttackAction(float dashSpeed = 10.0f, float attackTimeOffset = 0.5f);
    bool execute(Enemy* enemy, const vec2& pos, const vec2& dir) override;
    void update(Enemy* enemy, float dt) override;
    std::string getName() const override { return "Slide"; }
    
private:
    float dashSpeed;
    float attackTimeOffset;
    
    // Per-enemy state tracking
    struct SlideState {
        vec2 attackDir;
        float attackTimer;
        bool isSliding;
    };
    std::unordered_map<Enemy*, SlideState> slideStates;
    
    void updateSlideState(Enemy* enemy, float dt);
};

#endif
