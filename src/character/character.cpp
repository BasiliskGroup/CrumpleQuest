#include "character/character.h"

Character::Character(int health, Node2D* node, Weapon* weapon) : health(health), node(node), weapon(weapon) {}

Character::~Character() {
     // dont delete weapon
    delete node;
}

void Character::onDamage(int damage) {
    health -= damage;
}

void Character::onDeath() {
    delete this;
}

void Character::move(float dt) {
    vec3 velocity = node->getVelocity();
    velocity = (float) (1 - 10 * dt) * velocity;
    node->setVelocity(velocity);
}