#include "character/moveActions.h"
#include "character/enemy.h"

void NormalMoveAction::execute(Enemy* enemy, const vec2& destination) {
    vec2 enemyPos = enemy->getPosition();
    vec2 direction = destination - enemyPos;
    enemy->getMoveDir() = direction;
}

JumpMoveAction::JumpMoveAction(float cooldownTime, float dashSpeed) 
    : cooldownTime(cooldownTime), dashSpeed(dashSpeed) {
}

void JumpMoveAction::execute(Enemy* enemy, const vec2& destination) {
    // Initialize or update jump state for this enemy
    JumpState& state = jumpStates[enemy];
    
    // If on cooldown, update destination but don't jump yet
    if (state.isOnCooldown) {
        state.targetDestination = destination;
        return;
    }
    
    // Cooldown complete, perform jump
    vec2 enemyPos = enemy->getPosition();
    vec2 toTarget = destination - enemyPos;
    float dist = glm::length(toTarget);
    
    if (dist > 1e-6f) {
        // Calculate dash direction and set velocity directly
        vec2 dashDir = glm::normalize(toTarget);
        vec3 currentVel = enemy->getVelocity();
        vec3 dashVel = vec3(dashDir.x * dashSpeed, dashDir.y * dashSpeed, currentVel.z);
        
        // Set velocity directly and mark as unstable for sliding
        enemy->setVelocity(dashVel);
        enemy->getUnstable() = true;
        
        // Start cooldown
        state.targetDestination = destination;
        state.cooldownTimer = cooldownTime;
        state.isOnCooldown = true;
    }
}

void JumpMoveAction::update(Enemy* enemy, float dt) {
    updateJumpState(enemy, dt);
}

void JumpMoveAction::updateJumpState(Enemy* enemy, float dt) {
    auto it = jumpStates.find(enemy);
    if (it == jumpStates.end()) {
        return;  // No state for this enemy
    }
    
    JumpState& state = it->second;
    
    if (state.isOnCooldown) {
        // Count down cooldown timer
        state.cooldownTimer -= dt;
        if (state.cooldownTimer <= 0.0f) {
            // Cooldown complete, ready for next jump
            state.isOnCooldown = false;
            
            // If there's a pending destination, jump to it immediately
            vec2 enemyPos = enemy->getPosition();
            vec2 toTarget = state.targetDestination - enemyPos;
            float dist = glm::length(toTarget);
            
            if (dist > 1e-6f) {
                vec2 dashDir = glm::normalize(toTarget);
                vec3 currentVel = enemy->getVelocity();
                vec3 dashVel = vec3(dashDir.x * dashSpeed, dashDir.y * dashSpeed, currentVel.z);
                
                enemy->setVelocity(dashVel);
                enemy->getUnstable() = true;
                
                // Start cooldown again
                state.cooldownTimer = cooldownTime;
                state.isOnCooldown = true;
            }
        }
    }
}
