#include "character/enemy.h"
#include "game/game.h"


Enemy::Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai, float radius, vec2 scale) 
    : Character(game, health, speed, node, side, weapon, "Enemy", radius, scale), ai(ai), path() 
{
    animator = new Animator(game->getEngine(), node, game->getAnimation("player_idle"));
    animator->setFrameRate(8);
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move(const vec2& playerPos, float dt) {
    animator->update();

    if (glm::length2(moveDir) < 0.01) {
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

    // we have direct line of sight
    if (path.size() == 1) {
        attack(playerPos, dt); 
    }

    moveDir = path[0] - getPosition();

    // moveDir = playerPos - getPosition(); // old - direct line pathing
    Character::move(dt);
}

void Enemy::attack(const vec2& playerPos, float dt) {
    if (weapon == nullptr) return;


}