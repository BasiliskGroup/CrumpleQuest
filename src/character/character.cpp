#include "character/character.h"
#include "weapon/weapon.h"


Character::Character(int health, float speed, Node2D* node, Weapon* weapon, std::string team) : health(health), speed(speed), node(node), weapon(weapon), team(team) {}

Character::~Character() {
    delete weapon; weapon = nullptr;
    delete node; node = nullptr;
}

void Character::onDamage(int damage) {
    health -= damage;
}

void Character::onDeath() {
    delete this;
}

void Character::move(float dt) {
    if (glm::length2(moveDir) < EPSILON) {
        node->setVelocity( (float) (1 - 20 * dt) * node->getVelocity());
        return;
    }

    moveDir = glm::normalize(moveDir);

    vec3 current = node->getVelocity();
    vec3 accelVec = vec3(moveDir.x, moveDir.y, 0) * (float)this->accel * dt;

    // Compute the hypothetical new velocity
    vec3 trial = current + accelVec;
    float speed = glm::length(trial);

    if (speed > this->speed) {
        vec3 clamped = (trial / speed) * this->speed;
        accelVec = clamped - current;
    }

    // Apply only the allowed delta
    node->setVelocity(current + accelVec);
}