#include "character/character.h"

Character::Character(int health, float speed, Node2D* node, Weapon* weapon) : health(health), speed(speed), node(node), weapon(weapon) {}

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
    velocity = (float) (1 - 20 * dt) * velocity;
    node->setVelocity(velocity);
}