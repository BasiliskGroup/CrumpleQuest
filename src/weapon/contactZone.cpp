#include "weapon/contactZone.h"

ContactZone::ContactZone(Character* owner, Node2D::Params node, int damage, float life, bool friendlyDamage, bool selfDamage) :
    DamageZone(owner, node, { damage, life, friendlyDamage, selfDamage })
{}

bool ContactZone::update(float dt) {
    return DamageZone::update(dt);
}