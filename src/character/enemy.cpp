#include "character/enemy.h"

Enemy::Enemy(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai) 
    : Character(health, speed, node, side, weapon, "Enemy"), ai(ai) 
{}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move(const vec2& playerPos, float dt) {
    moveDir = playerPos - getPosition();
    Character::move(dt);
}