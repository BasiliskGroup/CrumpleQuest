#include "character/enemy.h"

Enemy::Enemy(int health, float speed, Node2D* node, Weapon* weapon, AI* ai) : Character(health, speed, node, weapon), ai(ai) {

}

Enemy::~Enemy() {
    // Probably don't delete the weapon or ai? 
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move(const vec2& playerPos, float dt) {
    Character::move(dt);

    vec3 dir = { playerPos - getPosition(), 0 };
    if (glm::length2(dir) < EPSILON) return;
    dir = (float) speed * glm::normalize(dir);

    setVelocity(dir);
}