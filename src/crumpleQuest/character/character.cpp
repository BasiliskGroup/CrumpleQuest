#include "crumpleQuest/character/character.h"

Character::Character(int health, Weapon* weapon) {
    this->health = health;
    this->weapon = weapon;
}

void Character::onDamage(int damage) {
    health -= damage;
}