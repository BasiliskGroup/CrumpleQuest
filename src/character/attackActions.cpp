#include "character/attackActions.h"
#include "character/enemy.h"
#include "weapon/weapon.h"
#include "resource/animation.h"
#include <unordered_map>

bool NormalAttackAction::execute(Enemy* enemy, const vec2& pos, const vec2& dir) {
    Weapon* weapon = enemy->getWeapon();
    if (weapon == nullptr) return false;
    if (!weapon->isReady()) return false;
    return weapon->attack(pos, dir);
}

BurstAttackAction::BurstAttackAction(int numShots, float delayBetweenShots) 
    : numShots(numShots), delayBetweenShots(delayBetweenShots) {
}

bool BurstAttackAction::execute(Enemy* enemy, const vec2& pos, const vec2& dir) {
    Weapon* weapon = enemy->getWeapon();
    if (weapon == nullptr) return false;
    if (!weapon->isReady()) return false;
    
    // Fire first projectile immediately
    bool firstShot = weapon->attack(pos, dir);
    if (!firstShot) return false;
    
    // Queue remaining projectiles with delays
    for (int i = 0; i < numShots - 1; ++i) {
        Enemy::PendingShot shot;
        shot.pos = pos;
        shot.dir = dir;
        shot.timer = delayBetweenShots * static_cast<float>(i + 1);
        shot.weapon = weapon;
        enemy->getPendingShots().push_back(shot);
    }
    
    return true;
}

void BurstAttackAction::update(Enemy* enemy, float dt) {
    // Burst action doesn't need per-frame updates
    // The pending shots are handled in Enemy::move()
}

SlideAttackAction::SlideAttackAction(float dashSpeed, float attackTimeOffset)
    : dashSpeed(dashSpeed), attackTimeOffset(attackTimeOffset) {
}

bool SlideAttackAction::execute(Enemy* enemy, const vec2& pos, const vec2& dir) {
    Weapon* weapon = enemy->getWeapon();
    if (weapon == nullptr) return false;
    if (!weapon->isReady()) return false;
    
    // Initialize slide state
    SlideState& state = slideStates[enemy];
    state.attackDir = dir;   // Direction toward player
    state.attackTimer = attackTimeOffset;
    state.isSliding = true;
    
    // Start attack animation immediately
    Animation* attackAnim = enemy->attackAnimation;
    if (attackAnim != nullptr) {
        float timePerFrame = 1.0f / 8.0f;  // Match the animator's frame rate
        unsigned int numFrames = attackAnim->getNumberFrames();
        float animationDuration = numFrames * timePerFrame;
        enemy->getAttacking() = animationDuration;
        
        // Explicitly set the animation on the animator to ensure it's used
        Animator* animator = enemy->getAnimator();
        if (animator != nullptr) {
            animator->setAnimation(attackAnim);
            // Reset frame rate to normal (8 fps) for attack animation
            animator->setFrameRate(8.0f);
        }
    }
    
    // Dash forward in attack direction using velocity
    vec3 currentVel = enemy->getVelocity();
    vec3 dashVel = vec3(dir.x * dashSpeed, dir.y * dashSpeed, currentVel.z);
    
    // Set velocity directly and mark as unstable for sliding
    enemy->setVelocity(dashVel);
    enemy->getUnstable() = true;
    
    // Set weapon cooldown (attack sequence has started)
    weapon->setCooldown(weapon->getMaxCooldown());
    
    return true;
}

void SlideAttackAction::update(Enemy* enemy, float dt) {
    updateSlideState(enemy, dt);
}

void SlideAttackAction::updateSlideState(Enemy* enemy, float dt) {
    auto it = slideStates.find(enemy);
    if (it == slideStates.end()) {
        return;  // No state for this enemy
    }
    
    SlideState& state = it->second;
    
    if (state.isSliding) {
        // Count down attack timer
        state.attackTimer -= dt;
        if (state.attackTimer <= 0.0f) {
            // Time to attack - use createDamageZone to create damage zone (cooldown already set)
            Weapon* weapon = enemy->getWeapon();
            if (weapon != nullptr) {
                // Attack at current position in the stored attack direction
                vec2 enemyPos = enemy->getPosition();
                vec2 attackPos = enemyPos + state.attackDir * enemy->getRadius() * 2.0f;
                weapon->createDamageZone(attackPos, state.attackDir);
            }
            
            // Slide attack complete
            state.isSliding = false;
            slideStates.erase(it);
        }
    }
}
