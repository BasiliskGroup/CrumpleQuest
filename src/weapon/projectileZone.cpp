#include "weapon/projectileZone.h"

ProjectileZone::ProjectileZone(Character* owner, Node2D::Params node, int damage, float life, int ricochet, bool friendlyDamage, bool selfDamage) :
    DamageZone(owner, node, { damage, life, friendlyDamage, selfDamage }),
    ricochet(ricochet)
{}

bool ProjectileZone::update(float dt) {
    if (DamageZone::update(dt) == false) return false;

    // check for expire on wall hit
    return true;
}