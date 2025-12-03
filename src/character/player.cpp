#include "character/player.h"
#include "weapon/weapon.h"
#include "game/game.h"

Player::Player(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 scale)
    : Character(game, health, speed, node, side, weapon, "Ally", radius, scale)
{
    this->accel = 30;
    weaponNode = new Node2D(node, { .mesh=game->getMesh("quad"), .material=game->getMaterial("knight"), .scale={1, 1}});
    weaponNode->setLayer(0.1f);

    animator = new Animator(node->getEngine(), node, game->getAnimation("player_idle"));
    animator->setFrameRate(8);

    weaponAnimator = new Animator(node->getEngine(), weaponNode, game->getAnimation("pencil_run"));
    weaponAnimator->setFrameRate(8);
}

void Player::onDamage(int damage) {
    Character::onDamage(damage);
    std::cout << "Player Health: " << health << std::endl;
}

void Player::move(float dt) {
    // if menu open, do nothing
    if (MenuManager::Get().hasActiveMenu()) {
        return;
    }

    // Update animations
    animator->update();
    weaponAnimator->update();

    // actual movement
    Keyboard* keys = node->getEngine()->getKeyboard();

    if (attacking > 0.0) {
        animator->setAnimation(game->getAnimation("player_attack"));
        weaponAnimator->setAnimation(game->getAnimation("pencil_attack"));
        
        unsigned int currentFrame = animator->getCurrentFrame();
        
        // Play sound on frame 0 when last frame was non-zero (loop) OR when we weren't attacking before (start)
        if (currentFrame == 0 && (lastAttackFrame != 0 || lastAttackFrame == 0xFFFFFFFF)) {
            audio::SFXPlayer::Get().Play("woosh");
            lastAttackFrame = 0; // Mark as played for this loop
        } else {
            lastAttackFrame = currentFrame;
        }
        
        attacking -= node->getEngine()->getDeltaTime();
    }
    else {
        lastAttackFrame = 0xFFFFFFFF; // Special value to indicate we're not attacking
        if (keys->getPressed(GLFW_KEY_W) || keys->getPressed(GLFW_KEY_D) || keys->getPressed(GLFW_KEY_A) || keys->getPressed(GLFW_KEY_S)) {
            animator->setAnimation(game->getAnimation("player_run"));
            weaponAnimator->setAnimation(game->getAnimation("pencil_run"));
        }
        else {
            animator->setAnimation(game->getAnimation("player_idle"));
            weaponAnimator->setAnimation(game->getAnimation("pencil_idle"));
        }
    }

    if (keys->getPressed(GLFW_KEY_A)) {
        node->setScale({scale.x, scale.y});
    }
    if (keys->getPressed(GLFW_KEY_D)) {
        node->setScale({-scale.x, scale.y});
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
    vec2 offset = radius / 2 * dir + getPosition();
    weapon->attack(offset, dir);
    attacking = 1.0 / 8.0 * 4.0;
}
