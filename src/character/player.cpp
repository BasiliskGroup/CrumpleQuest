#include "character/player.h"

Player::Player(int health, Node2D* node, Weapon* weapon) : Character(health, node, weapon) {}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
}

void Player::move(float dt) {
    Character::move(dt);

    Keyboard* keys = node->getEngine()->getKeyboard();

    vec2 dir {
        keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_D),
        keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S),
    };

    // if the player isn't pressing keys
    if (glm::length2(dir) < EPSILON) return;

    node->setVelocity(node->getVelocity() + vec3{ dir.x, dir.y, 0 });
}