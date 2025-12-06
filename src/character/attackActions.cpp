#include "character/attackActions.h"
#include "character/enemy.h"
#include "weapon/weapon.h"

bool NormalAttackAction::execute(Enemy* enemy, const vec2& pos, const vec2& dir) {
    Weapon* weapon = enemy->getWeapon();
    if (weapon == nullptr) return false;
    if (!weapon->isReady()) return false;
    return weapon->attack(pos, dir);
}

BurstAttackAction::BurstAttackAction(int numShots, float delayBetweenShots) 
    : numShots(numShots), delayBetweenShots(delayBetweenShots) {
}

bool BurstAttackAction::execute(Enemy* enemy, const vec2& pos, const vec2& dir) {
    Weapon* weapon = enemy->getWeapon();
    if (weapon == nullptr) return false;
    if (!weapon->isReady()) return false;
    
    // Fire first projectile immediately
    bool firstShot = weapon->attack(pos, dir);
    if (!firstShot) return false;
    
    // Queue remaining projectiles with delays
    for (int i = 0; i < numShots - 1; ++i) {
        Enemy::PendingShot shot;
        shot.pos = pos;
        shot.dir = dir;
        shot.timer = delayBetweenShots * static_cast<float>(i + 1);
        shot.weapon = weapon;
        enemy->getPendingShots().push_back(shot);
    }
    
    return true;
}

void BurstAttackAction::update(Enemy* enemy, float dt) {
    // Burst action doesn't need per-frame updates
    // The pending shots are handled in Enemy::move()
}
