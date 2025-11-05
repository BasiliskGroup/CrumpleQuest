#include "character/player.h"

Player::Player(int health, float speed, Node2D* node, Weapon* weapon) : Character(health, speed, node, weapon, "Ally") {}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
}

void Player::move(float dt) {
    Character::move(dt);

    Keyboard* keys = node->getEngine()->getKeyboard();

    vec2 dir = {
        keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A),
        keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)
    };

    // if the player isn't pressing keys
    if (glm::length2(dir) < EPSILON) return;

    dir = glm::normalize(dir);

    node->setVelocity((float) this->speed * vec3{ dir.x, dir.y, 0 });
    // node->setVelocity({1, 1, 0});
}