#include "weapon/damageZone.h"
#include "levels/levels.h"

DamageZone::DamageZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir) : 
    Node2D(owner->getSide()->getScene(), node),
    owner(owner),
    damage(params.damage),
    life(params.life),
    vel(params.speed * dir),
    radius(params.radius),
    friendlyDamage(params.friendlyDamage),
    selfDamage(params.selfDamage),
    onExpire(params.onExpire),
    onHit(params.onHit)
{
    this->setPosition(pos);
}

bool DamageZone::update(float dt) {
    this->setPosition(this->getPosition() + dt * vel);

    life -= dt;
    if (life > 0) return true;

    if (onExpire) onExpire();
    return false;
}

bool DamageZone::hit(Character* other) {
    if (!other) return false;

    bool isSelf = (owner == other);
    bool isFriendly = (owner->getTeam() == other->getTeam());

    // skip damage
    if ((isSelf && !selfDamage) || (isFriendly && !friendlyDamage)) return false; 

    if (onHit) onHit(other);
    other->onDamage(damage);
    return true;
}