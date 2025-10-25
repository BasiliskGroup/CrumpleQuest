#include "crumpleQuest/character/enemy.h"

Enemy::Enemy(int health, Weapon* weapon, AI* ai) : Character(health, weapon), ai(ai) {

}

Enemy::~Enemy() {
    // Probably don't delete the weapon or ai? 
}

void Enemy::onDamage(int damage) {
    Character::onDamage(damage);
}

void Enemy::move() {
    
}