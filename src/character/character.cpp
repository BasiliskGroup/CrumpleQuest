#include "character/character.h"

Character::Character(int health, float speed, Node2D* node, Weapon* weapon, std::string team) : health(health), speed(speed), node(node), weapon(weapon), team(team) {}

Character::~Character() {
    delete weapon;
    delete node;
}

void Character::onDamage(int damage) {
    health -= damage;
}

void Character::onDeath() {
    delete this;
}

void Character::move(float dt) {
    vec3 velocity = getVelocity();
    velocity = (float) (1 - 20 * dt) * velocity;
    setVelocity(velocity);
}