#include "crumpleQuest/character/player.h"

Player::Player(int health, Weapon* weapon) : Character(health, weapon) {}

void Player::onDamage(int damage) {
    health -= damage;
}

void Player::move() {
    
}