#include "weapon/damageZone.h"

DamageZone::DamageZone(Character* owner, Node2D* hitbox, Params params) :
    owner(owner),
    hitbox(hitbox),
    damage(params.damage),
    life(params.life),
    friendlyDamage(params.friendlyDamage),
    selfDamage(params.selfDamage),
    onExpire(params.onExpire),
    onHit(params.onHit)
{}

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