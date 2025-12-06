#include "character/enemy.h"
#include "character/behavior.h"
#include "game/game.h"
#include "weapon/weapon.h"
#include "audio/sfx_player.h"


Enemy::Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai, float radius, vec2 scale, std::string hitSound, float attackDelay) 
    : Character(game, health, speed, node, side, weapon, "Enemy", radius, scale, hitSound), ai(ai), path(), attackDelay(attackDelay), attackDelayTimer(0.0f), attackPending(false)
{
    animator = new Animator(game->getEngine(), node, game->getAnimation("player_idle"));
    animator->setFrameRate(8);
    radius = glm::length(node->getScale() * node->getColliderScale());
    
    // Initialize behaviorSelector to default to returning idle behavior
    behaviorSelector = [](const vec2&, float) -> Behavior* {
        return BehaviorRegistry::getBehavior("Idle");
    };

    attackAction = [this](const vec2& pos, const vec2& dir) {
        if(this->weapon == nullptr) return false;
        if(!this->weapon->isReady()) return false;
        return this->weapon->attack(pos, dir);
    };
    
    // Default moveAction: move directly toward destination
    moveAction = [this](const vec2& destination) {
        vec2 enemyPos = this->getPosition();
        vec2 direction = destination - enemyPos;
        this->getMoveDir() = direction;
    };
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::onDeath() {
    std::cout << "Enemy died" << std::endl;
    Character::onDeath();
}

void Enemy::updateStatus(const vec2& playerPos) {
    // Update line of sight status
    statusHasLineOfSight = hasLineOfSight(getPosition(), playerPos);
    
    // Update weapon ready status
    statusWeaponReady = (weapon != nullptr) && weapon->isReady();
    
    // Update number of enemies on the same side (count alive enemies)
    statusNumEnemiesOnSide = 0;
    if (side != nullptr) {
        auto& enemies = side->getEnemies();
        for (Enemy* e : enemies) {
            if (e != nullptr && !e->isDead()) {
                statusNumEnemiesOnSide++;
            }
        }
    }
    
    // Update attack capability status (weapon ready, player is in range, and has line of sight)
    bool inRange = false;
    if (weapon != nullptr) {
        float distance = glm::length(playerPos - getPosition());
        inRange = distance <= weapon->getRange();
    }
    statusCanAttack = statusWeaponReady && inRange;
    
    // Update attacking status (currently playing attack animation)
    statusIsAttacking = attacking > 0.0f;
    
    // Update path status
    statusHasPath = path.size() > 0;
}

void Enemy::move(const vec2& playerPos, float dt) {
    animator->update();
    
    // Update weapon cooldown
    if (weapon != nullptr) {
        weapon->update(dt);
    }
    
    // Update attack animation timer
    if (attacking > 0.0f) {
        attacking -= dt;
    }
    
    // Update attack delay timer
    if (attackPending && attackDelayTimer > 0.0f) {
        attackDelayTimer -= dt;
        if (attackDelayTimer <= 0.0f) {
            // Delay complete, execute the attack using attackAction
            vec2 offset = radius * pendingAttackDir + getPosition();
            bool attackSuccessful = attackAction(offset, pendingAttackDir);
            attackPending = false;
        }
    }
    
    // Process pending shots (for multi-shot attacks)
    for (int i = pendingShots.size() - 1; i >= 0; i--) {
        pendingShots[i].timer -= dt;
        if (pendingShots[i].timer <= 0.0f) {
            // Time to fire this shot - use createDamageZone to bypass cooldown
            if (pendingShots[i].weapon != nullptr) {
                pendingShots[i].weapon->createDamageZone(pendingShots[i].pos, pendingShots[i].dir);
            }
            pendingShots.erase(pendingShots.begin() + i);
        }
    }

    // Use behaviorSelector to get the behavior, then update it
    Behavior* selectedBehavior = behaviorSelector(playerPos, dt);
    if (selectedBehavior != nullptr) {
        selectedBehavior->update(this, playerPos, dt);
    } else {
        // Fallback if no behavior selected
        moveDir = { 0, 0 };
        path.clear();
    }

    // Set animation based on state: attack > movement
    if (attacking > 0.0f && attackAnimation != nullptr) {
        animator->setAnimation(attackAnimation);
    }
    else if (glm::length2(moveDir) < 0.01) {
        animator->setAnimation(idleAnimation);
    }
    else {
        animator->setAnimation(runAnimation);
    }

    // Flip sprite based on movement direction
    if (moveDir[0] > 0) {
        node->setScale({-scale.x, scale.y});
    }
    else {
        node->setScale({scale.x, scale.y});
    }

    Character::move(dt);
}

void Enemy::attack(const vec2& playerPos, float dt) {
    if (weapon == nullptr) return;
    if (!weapon->isReady()) return;  // Can't attack if weapon is on cooldown

    vec2 dir = playerPos - getPosition();
    if (glm::length2(dir) < 1e-6f) return;
    dir = glm::normalize(dir);

    // If there's an attack delay, start the animation and schedule the attack
    if (attackDelay > 0.0f && !attackPending) {
        // Start attack animation immediately
        if (attackAnimation != nullptr) {
            float timePerFrame = 1.0f / 8.0f;  // Match the animator's frame rate
            unsigned int numFrames = attackAnimation->getNumberFrames();
            attacking = numFrames * timePerFrame;
        }
        
        // Schedule the actual weapon attack after delay
        attackPending = true;
        attackDelayTimer = attackDelay;
        pendingAttackPos = playerPos;
        pendingAttackDir = dir;
    } else {
        // No delay or delay already in progress, attack immediately (if no delay)
        if (attackDelay <= 0.0f) {
            vec2 offset = 2 * radius * dir + getPosition();
            bool attackSuccessful = attackAction(offset, dir);
            
            // If attack was successful (weapon was off cooldown), set attack animation duration
            if (attackSuccessful && attackAnimation != nullptr) {
                float timePerFrame = 1.0f / 8.0f;  // Match the animator's frame rate
                unsigned int numFrames = attackAnimation->getNumberFrames();
                attacking = numFrames * timePerFrame;
            }
        }
    }
}

void Enemy::setBehavior(const std::string& behaviorName) {
    behavior = BehaviorRegistry::getBehavior(behaviorName);
}

void Enemy::updateNode(Node2D* newNode) {
    setNode(newNode);
    if (animator != nullptr) {
        animator->setNode(newNode);
    }
}