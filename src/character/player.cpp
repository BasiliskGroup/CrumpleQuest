#include "character/player.h"
#include "weapon/weapon.h"
#include "game/game.h"
#include "audio/sfx_player.h"

Player::Player(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 scale)
    : Character(game, health, speed, node, side, weapon, "Ally", radius, scale, "hit-player")
{
    this->accel = 30;
    weaponNode = side->getWeaponNode();
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
    
    // Update weapon cooldown
    if (weapon != nullptr) {
        weapon->update(dt);
    }
    
    // Update attack animation timer
    if (attacking > 0.0f) {
        attacking -= dt;
    }

    // actual movement
    Keyboard* keys = node->getEngine()->getKeyboard();

    // Set animation based on state: attack > movement
    if (attacking > 0.0f) {
        Animation* attackAnim = game->getAnimation("player_attack");
        Animation* weaponAttackAnim = game->getAnimation("pencil_attack");
        
        if (attackAnim != nullptr) {
            animator->setAnimation(attackAnim);
        }
        if (weaponAttackAnim != nullptr) {
            weaponAnimator->setAnimation(weaponAttackAnim);
        }
        
        unsigned int currentFrame = animator->getCurrentFrame();
        
        // Play sound on frame 0 when last frame was non-zero (loop) OR when we weren't attacking before (start)
        if (currentFrame == 0 && (lastAttackFrame != 0 || lastAttackFrame == 0xFFFFFFFF)) {
            audio::SFXPlayer::Get().Play("woosh");
            lastAttackFrame = 0; // Mark as played for this loop
        } else {
            lastAttackFrame = currentFrame;
        }
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
    
    // Update weapon node position and scale
    weaponNode->setScale(node->getScale());
    weaponNode->setPosition(node->getPosition());

    // Handle attack input
    if (weapon == nullptr) return;
    Mouse* mouse = node->getEngine()->getMouse();
    if (mouse->getClicked() == false) return;

    vec2 pos = { 
        mouse->getWorldX(node->getScene()->getCamera()) * 8.0 / 4.6153, 
        mouse->getWorldY(node->getScene()->getCamera()) * 4.5 / 3.492
    };

    vec2 dir = pos - getPosition();
    if (glm::length2(dir) < 1e-6f) return;

    dir = glm::normalize(dir);
    vec2 offset = radius * dir + getPosition();
    
    bool attackSuccessful = weapon->attack(offset, dir);
    
    // If attack was successful (weapon was off cooldown), set attack animation duration
    if (attackSuccessful) {
        Animation* attackAnim = game->getAnimation("player_attack");
        if (attackAnim != nullptr) {
            // Calculate attack animation duration: number of frames * time per frame
            // Animator frame rate is 8 fps, so timePerFrame = 1.0 / 8 = 0.125
            float timePerFrame = 1.0f / 8.0f;  // Match the animator's frame rate
            unsigned int numFrames = attackAnim->getNumberFrames();
            attacking = numFrames * timePerFrame;
        }
    }
}

void Player::setNodes(Node2D* node, Node2D* weapon) { 
    this->node = node; 
    this->animator->setNode(node);

    this->weaponNode = weapon;
    this->weaponAnimator->setNode(weapon);
}