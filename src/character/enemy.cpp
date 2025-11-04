#include "character/enemy.h"

Enemy::Enemy(int health, Node2D* node, Weapon* weapon, AI* ai) : Character(health, node, weapon), ai(ai) {

}

Enemy::~Enemy() {
    // Probably don't delete the weapon or ai? 
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move(float dt) {
    
}