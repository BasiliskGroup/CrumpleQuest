#include "weapon/damageZone.h"

DamageZone::DamageZone(Character* owner, Node2D::Params node, Params params) : 
    Node2D(owner->getNode()->getScene(), node),
    owner(owner),
    damage(params.damage),
    life(params.life),
    friendlyDamage(params.friendlyDamage),
    selfDamage(params.selfDamage),
    onExpire(params.onExpire),
    onHit(params.onHit)
{}

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