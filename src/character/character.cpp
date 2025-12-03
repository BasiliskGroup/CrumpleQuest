#include "levels/levels.h"
#include "weapon/weapon.h"
#include "game/game.h"
#include "audio/sfx_player.h"


Character::Character(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, std::string team, float radius, vec2 scale, std::string damageSound) : 
    game(game),
    health(health), 
    speed(speed), 
    node(node), 
    side(side), 
    weapon(weapon), 
    team(team),
    radius(radius),
    scale(scale),
    damageSound(damageSound)
{
    node->setManifoldMask(1, 1, 0);
}

Character::~Character() {
    delete weapon; weapon = nullptr;
    delete node; node = nullptr;
    side = nullptr;
}

void Character::onDamage(int damage) {
    if (itime > 0) return;
    if (isDead()) return; // Don't process damage if already dead
    
    // Play damage sound
    if (!damageSound.empty()) {
        audio::SFXPlayer::Get().Play(damageSound);
    }
    
    health -= damage;
    itime = 0.2;
}

void Character::onDeath() {
    
}

void Character::move(float dt) {
    // reduce invincibility frames
    itime = glm::clamp(itime, 0.0f, itime - dt);

    // slow modement if we are not being controlled
    if (glm::length2(moveDir) < EPSILON) {
        node->setVelocity( (float) (1 - 20 * dt) * node->getVelocity());
        return;
    }

    moveDir = glm::normalize(moveDir);

    vec3 current = node->getVelocity();
    vec3 accelVec = vec3(moveDir.x, moveDir.y, 0) * (float) this->accel * dt;

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