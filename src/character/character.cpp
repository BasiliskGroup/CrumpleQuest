#include "levels/levels.h"
#include "levels/paperMesh.h"
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

    vec3 current = node->getVelocity();
    float currentSpeed = glm::length(current);
    
    // Check if unstable character should become stable (velocity <= max speed)
    if (unstable && currentSpeed <= speed) {
        unstable = false;
    }

    // If unstable, ignore moveDir and just apply friction (allow sliding)
    if (unstable) {
        // Apply friction to slow down over time
        node->setVelocity((float)(1 - 2 * dt) * node->getVelocity());
        return;
    }

    // slow modement if we are not being controlled
    if (glm::length2(moveDir) < EPSILON) {
        node->setVelocity( (float) (1 - 20 * dt) * node->getVelocity());
        return;
    }

    moveDir = glm::normalize(moveDir);

    // Normal movement acceleration
    vec3 accelVec = vec3(moveDir.x, moveDir.y, 0) * (float) this->accel * dt;

    // Compute the hypothetical new velocity
    vec3 trial = current + accelVec;
    float trialSpeed = glm::length(trial);

    // Clamp to max speed
    if (trialSpeed > this->speed) {
        vec3 clamped = (trial / trialSpeed) * this->speed;
        accelVec = clamped - current;
    }

    // Apply only the allowed delta
    node->setVelocity(current + accelVec);
}

bool Character::hasLineOfSight(const vec2& start, const vec2& end) const {
    // Get paper from game
    if (game == nullptr) {
        return true; // No game reference, assume line of sight (shouldn't happen)
    }
    
    Paper* paper = game->getPaper();
    if (paper == nullptr) {
        return true; // No paper, assume line of sight (shouldn't happen)
    }
    
    // Determine which PaperMesh corresponds to this character's side
    PaperMesh* paperMesh = nullptr;
    if (side == paper->getFirstSide()) {
        paperMesh = paper->paperMeshes.first;
    } else if (side == paper->getSecondSide()) {
        paperMesh = paper->paperMeshes.second;
    }
    
    // If we couldn't find the matching mesh, assume no line of sight to be safe
    if (paperMesh == nullptr) {
        return false;
    }
    
    // Use the PaperMesh's hasLineOfSight function
    return paperMesh->hasLineOfSight(start, end);
}

PaperMesh* Character::getPaperMeshForSide() const {
    // Get paper from game
    if (game == nullptr) {
        return nullptr;
    }
    
    Paper* paper = game->getPaper();
    if (paper == nullptr) {
        return nullptr;
    }
    
    // Determine which PaperMesh corresponds to this character's side
    if (side == paper->getFirstSide()) {
        return paper->paperMeshes.first;
    } else if (side == paper->getSecondSide()) {
        return paper->paperMeshes.second;
    }
    
    return nullptr;
}