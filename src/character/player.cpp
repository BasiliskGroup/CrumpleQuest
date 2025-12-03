#include "character/player.h"
#include "weapon/weapon.h"


Player::Player(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, std::unordered_map<std::string, Animation*>* animations, Node2D* weaponNode)
    : Character(health, speed, node, side, weapon, "Ally")
{
    this->accel = 30;
    this->animations = animations;
    this->weaponNode = weaponNode;
    
    animator = new Animator(node->getEngine(), node, animations->at("player_idle"));
    animator->setFrameRate(6);

    weaponAnimator = new Animator(node->getEngine(), weaponNode, animations->at("pencil_run"));
    weaponAnimator->setFrameRate(6);
}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
    std::cout << "Player Health: " << health << std::endl;
}

void Player::move(float dt) {

    // Update animations
    animator->update();
    weaponAnimator->update();

    // actual movement
    Keyboard* keys = node->getEngine()->getKeyboard();

    if (keys->getPressed(GLFW_KEY_W) || keys->getPressed(GLFW_KEY_D) || keys->getPressed(GLFW_KEY_A) || keys->getPressed(GLFW_KEY_S)) {
        animator->setAnimation(animations->at("player_run"));
    }
    else {
        animator->setAnimation(animations->at("player_idle"));
    }

    if (keys->getPressed(GLFW_KEY_A)) {
        node->setScale({1, 1});
    }
    if (keys->getPressed(GLFW_KEY_D)) {
        node->setScale({-1, 1});
    }
    
    moveDir = {
        keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A),
        keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)
    };
    
    Character::move(dt);
    
    // attacking
    weaponNode->setScale(node->getScale());
    weaponNode->setPosition(node->getPosition());

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
