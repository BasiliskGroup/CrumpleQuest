#include "character/enemy.h"
#include "game/game.h"
#include "weapon/weapon.h"
#include "audio/sfx_player.h"


Enemy::Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai, float radius, vec2 scale, std::string hitSound) 
    : Character(game, health, speed, node, side, weapon, "Enemy", radius, scale, hitSound), ai(ai), path()
{
    animator = new Animator(game->getEngine(), node, game->getAnimation("player_idle"));
    animator->setFrameRate(8);
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
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

    if (moveDir[0] > 0) {
        node->setScale({-scale.x, scale.y});
    }
    else {
        node->setScale({scale.x, scale.y});
    }


    // no valid path, stay still TODO make idle behavior
    if (path.size() == 0) {
        moveDir = { 0, 0 };
        Character::move(dt);
        return;
    }

    while (glm::length2(path[0] - getPosition()) < finishRadius) {
        path.erase(path.begin());

        if (path.size() == 0) {
            moveDir = { 0, 0 };
            Character::move(dt);
            return;
        }
    }

    // we have direct line of sight if there are exactly 2 unique waypoints
    // (enemy position -> waypoint -> player position)
    bool hasLineOfSight = false;
    if (path.size() >= 2) {
        // Check if the first two waypoints are unique (not the same position)
        vec2 firstWaypoint = path[0];
        vec2 secondWaypoint = path[1];
        float epsilon = 1e-6f;
        if (glm::length2(firstWaypoint - secondWaypoint) > epsilon * epsilon) {
            hasLineOfSight = true;
        }
    } else {
        hasLineOfSight = true;
    }
    
    if (hasLineOfSight) {
        attack(playerPos, dt); 
    }

    moveDir = path[0] - getPosition();

    // moveDir = playerPos - getPosition(); // old - direct line pathing
    Character::move(dt);
}

void Enemy::attack(const vec2& playerPos, float dt) {
    if (weapon == nullptr) return;

    vec2 dir = playerPos - getPosition();
    if (glm::length2(dir) < 1e-6f) return;
    dir = glm::normalize(dir);

    vec2 offset = radius / 2 * dir + getPosition();
    bool attackSuccessful = weapon->attack(offset, dir);
    
    // If attack was successful (weapon was off cooldown), set attack animation duration
    if (attackSuccessful && attackAnimation != nullptr) {
        // Calculate attack animation duration: number of frames * time per frame
        // Animator frame rate is 8 fps, so timePerFrame = 1.0 / 8 = 0.125
        float timePerFrame = 1.0f / 8.0f;  // Match the animator's frame rate
        unsigned int numFrames = attackAnimation->getNumberFrames();
        attacking = numFrames * timePerFrame;
    }
}

void Enemy::updateNode(Node2D* newNode) {
    setNode(newNode);
    if (animator != nullptr) {
        animator->setNode(newNode);
    }
}