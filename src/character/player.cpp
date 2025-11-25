#include "character/player.h"

Player::Player(int health, float speed, Node2D* node, Weapon* weapon) : Character(health, speed, node, weapon, "Ally") {}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
    std::cout << "Player Health: " << health << std::endl;
}

void Player::move(float dt) {
    Keyboard* keys = node->getEngine()->getKeyboard();

    moveDir = {
        keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A),
        keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)
    };

    Character::move(dt);
}
