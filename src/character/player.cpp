#include "character/player.h"
#include "weapon/weapon.h"
#include "game/game.h"
#include "audio/sfx_player.h"

Player::Player(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 scale)
    : Character(game, 6, speed, node, side, weapon, "Ally", radius, scale, "hit-player")
{

    this->health = health;
    this->accel = 30;
    weaponNode = side->getWeaponNode();
    weaponNode->setLayer(0.1f);

    animator = new Animator(node->getEngine(), node, game->getAnimation("player_idle"));
    animator->setFrameRate(8);

    weaponAnimator = new Animator(node->getEngine(), weaponNode, game->getAnimation("pencil_idle"));
    weaponAnimator->setFrameRate(8);

    setWeaponPencil();
}

Player::~Player() {
    delete animator;
    delete weaponAnimator;
}

void Player::onDamage(int damage) {
    Character::onDamage(damage);

    // End game if dead
    if (isDead())
    {
        MenuManager::Get().pushGameOverMenu();
    }
}

void Player::onDeath() {
    std::cout << "Player died" << std::endl;
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

    // Update dash timer
    if (dashTimer > 0.0f) {
        dashTimer -= dt;
    }

    // actual movement
    Keyboard* keys = node->getEngine()->getKeyboard();

    // Set animation based on state: attack > movement
    if (attacking > 0.0f) {
        if (attackAnimation != nullptr) {
            animator->setAnimation(attackAnimation);
        }
        if (weaponAttack != nullptr) {
            weaponAnimator->setAnimation(weaponAttack);
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
            animator->setAnimation(runAnimation);
            weaponAnimator->setAnimation(weaponRun);
        }
        else {
            animator->setAnimation(idleAnimation);
            weaponAnimator->setAnimation(weaponIdle);
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

    if (keys->getPressed(GLFW_KEY_LEFT_SHIFT) && shiftWasDown == false && dashTimer <= 0.0f) {
        dashTimer = maxDashTimer;
        unstable = true;
        node->setVelocity(node->getVelocity() + vec3(moveDir, 0) * 10.0f);
    }
    shiftWasDown = keys->getPressed(GLFW_KEY_LEFT_SHIFT);
    
    if (keys->getPressed(GLFW_KEY_1) && pencilAvailible) {
        setWeaponPencil();
    }
    if (keys->getPressed(GLFW_KEY_2) && stapleGunAvailible) {
        setWeaponStapleGun();
    }
    if (keys->getPressed(GLFW_KEY_3) && scissorAvailible) {
        setWeaponScissor();
    }

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
    
    // Check if weapon is a projectile weapon - if so, spawn at player position (no offset)
    // Otherwise (melee weapons), use offset
    ProjectileWeapon* projectileWeapon = dynamic_cast<ProjectileWeapon*>(weapon);
    vec2 attackPos;
    if (projectileWeapon != nullptr) {
        // Projectile weapons spawn directly at player position
        attackPos = getPosition();
    } else {
        // Melee weapons use offset
        attackPos = 2 * radius * dir + getPosition();
    }
    
    bool attackSuccessful = weapon->attack(attackPos, dir);
    
    // If attack was successful (weapon was off cooldown), set attack animation duration
    if (attackSuccessful) {
        Animation* attackAnim = attackAnimation;
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

void Player::setWeaponPencil() {
    idleAnimation = game->getAnimation("player_idle");
    runAnimation = game->getAnimation("player_run");
    attackAnimation = game->getAnimation("player_attack_pencil");
    weaponIdle = game->getAnimation("pencil_idle");
    weaponRun = game->getAnimation("pencil_run");
    weaponAttack = game->getAnimation("pencil_attack");

    animator->setAnimation(idleAnimation);
    weaponAnimator->setAnimation(weaponIdle);

    // Create and set MeleeWeapon for pencil
    // Use same stats as initial weapon from game.cpp
    float meleeRadius = 1.0f;
    if (weapon != nullptr) {
        delete weapon;
    }
    setWeapon(new MeleeWeapon(this, 
        { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={meleeRadius, meleeRadius}}, 
        { .damage=1, .life=0.2f, .radius=meleeRadius / 2.0f }, 
        0.5f,  // maxCooldown
        6.0f   // knockback
    ));
}

void Player::setWeaponStapleGun() {
    idleAnimation = game->getAnimation("player_idle");
    runAnimation = game->getAnimation("player_run");
    attackAnimation = game->getAnimation("player_attack_gun");
    weaponIdle = game->getAnimation("gun_idle");
    weaponRun = game->getAnimation("gun_run");
    weaponAttack = game->getAnimation("gun_attack");

    animator->setAnimation(idleAnimation);
    weaponAnimator->setAnimation(weaponIdle);

    // Create and set ProjectileWeapon for stapler
    float projectileRadius = 0.5f;
    if (weapon != nullptr) {
        delete weapon;
    }
    setWeapon(new ProjectileWeapon(this,
        { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={projectileRadius, 2.0f * projectileRadius}},
        { .damage=1, .life=5.0f, .speed=7.0f, .radius=projectileRadius / 2.0f },
        0.8f,  // maxCooldown
        { "stapleProjectile" },  // projectile materials
        0      // ricochet
    ));
}

void Player::setWeaponScissor() {
    idleAnimation = game->getAnimation("player_idle");
    runAnimation = game->getAnimation("player_run");
    attackAnimation = game->getAnimation("player_attack_pencil");
    weaponIdle = game->getAnimation("pencil_idle");
    weaponRun = game->getAnimation("pencil_run");
    weaponAttack = game->getAnimation("pencil_attack");

    animator->setAnimation(idleAnimation);
    weaponAnimator->setAnimation(weaponIdle);

    // Create and set MeleeWeapon for scissor (same stats as pencil)
    float meleeRadius = 1.0f;
    if (weapon != nullptr) {
        delete weapon;
    }
    setWeapon(new MeleeWeapon(this, 
        { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={meleeRadius, meleeRadius}}, 
        { .damage=1, .life=0.2f, .radius=meleeRadius / 2.0f }, 
        0.5f,  // maxCooldown
        6.0f   // knockback
    ));
}

void Player::addHealth(int amount) {
    health = glm::clamp(health + amount, 0, maxHealth);
}