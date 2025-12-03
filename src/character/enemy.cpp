#include "character/enemy.h"

Enemy::Enemy(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai) 
    : Character(health, speed, node, side, weapon, "Enemy"), ai(ai), path() 
{}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move(const vec2& playerPos, float dt) {
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

    moveDir = path[0] - getPosition();

    // moveDir = playerPos - getPosition(); // old - direct line pathing
    Character::move(dt);
}