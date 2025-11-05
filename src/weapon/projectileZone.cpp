#include "weapon/projectileZone.h"

ProjectileZone::ProjectileZone(Character* owner, Node2D* hitbox, int damage, float life, int ricochet, bool friendlyDamage, bool selfDamage) :
    DamageZone(owner, hitbox, damage, life, friendlyDamage, selfDamage),
    ricochet(ricochet)
{}

bool ProjectileZone::update(float dt) {
    if (DamageZone::update(dt) == false) return false;

    // check for expire on wall hit
    return true;
}