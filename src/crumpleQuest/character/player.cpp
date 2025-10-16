#include "crumpleQuest/character/player.h"

Player::Player(int health, Weapon* weapon) : Character(health, weapon) {}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
}

void Player::move() {
    
}