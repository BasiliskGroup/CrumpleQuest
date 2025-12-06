#ifndef MOVE_ACTIONS_H
#define MOVE_ACTIONS_H

#include "character/moveAction.h"
#include <unordered_map>

// Normal move action - moves directly toward destination
class NormalMoveAction : public MoveAction {
public:
    void execute(Enemy* enemy, const vec2& destination) override;
    std::string getName() const override { return "Normal"; }
};

// Jump move action - waits for cooldown, then dashes quickly toward destination using velocity
class JumpMoveAction : public MoveAction {
public:
    JumpMoveAction(float cooldownTime = 2.0f, float dashSpeed = 10.0f);
    void execute(Enemy* enemy, const vec2& destination) override;
    void update(Enemy* enemy, float dt) override;
    std::string getName() const override { return "Jump"; }
    
private:
    float cooldownTime;
    float dashSpeed;
    
    // Per-enemy state tracking
    struct JumpState {
        vec2 targetDestination;
        float cooldownTimer;
        bool isOnCooldown;
    };
    std::unordered_map<Enemy*, JumpState> jumpStates;
    
    void updateJumpState(Enemy* enemy, float dt);
};

#endif
