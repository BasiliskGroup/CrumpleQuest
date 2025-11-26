#include "character/player.h"
#include "weapon/weapon.h"


Player::Player(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon) 
    : Character(health, speed, node, side, weapon, "Ally") 
{}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
    std::cout << "Player Health: " << health << std::endl;
}

void Player::move(float dt) {
    // actual movement
    Keyboard* keys = node->getEngine()->getKeyboard();

    moveDir = {
        keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A),
        keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)
    };

    Character::move(dt);

    // attacking
    if (weapon == nullptr) return;
    Mouse* mouse = node->getEngine()->getMouse();
    if (mouse->getClicked() == false) return;

    vec2 pos = { 
        mouse->getWorldX(node->getScene()->getCamera()), 
        mouse->getWorldY(node->getScene()->getCamera())
    };

    vec2 dir = pos - getPosition();
    if (glm::length2(dir) < 1e-6f) return;

    dir = glm::normalize(dir);
    vec2 offset = radius * dir + getPosition();
    weapon->attack(offset, dir);
}
