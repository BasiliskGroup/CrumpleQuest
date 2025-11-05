#include "weapon/damageZone.h"

DamageZone::DamageZone(Character* owner, Node2D* hitbox, int damage, float life, bool friendlyDamage, bool selfDamage) :
    owner(owner),
    hitbox(hitbox),
    damage(damage),
    life(life),
    friendlyDamage(friendlyDamage),
    selfDamage(selfDamage)
{
    // create default functions
    onHit = [this](Character* hit) {};
    onExpire = [this]() {};
}

DamageZone::~DamageZone() {
    delete hitbox;
}

bool DamageZone::update(float dt) {
    life -= dt;
    if (life > 0) return true;

    if (onExpire) onExpire();
    return false;
}

void DamageZone::hit(Character* other) {
    if (!other) return;

    bool isSelf = (owner == other);
    bool isFriendly = (owner->getTeam() == other->getTeam());

    // skip damage
    if ((isSelf && !selfDamage) || (isFriendly && !friendlyDamage)) return; 

    if (onHit) onHit(other);
    other->onDamage(damage);
}