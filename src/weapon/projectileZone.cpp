#include "weapon/projectileZone.h"

ProjectileZone::ProjectileZone(Character* owner, Node2D* hitbox, int damage, float life, int ricochet=0, bool friendlyDamage=false, bool selfDamage=false) :
    DamageZone(owner, hitbox, damage, life, friendlyDamage, selfDamage),
    ricochet(ricochet)
{}

bool ProjectileZone::update(float dt) {
    if (DamageZone::update(dt) == false) return false;

    // check for expire on wall hit
    return true;
}