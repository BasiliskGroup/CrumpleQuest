#include "weapon/projectileZone.h"

ProjectileZone::ProjectileZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, int ricochet) :
    DamageZone(owner, node, params, pos, dir),
    ricochet(ricochet)
{}

bool ProjectileZone::update(float dt) {
    if (DamageZone::update(dt) == false) return false;

    // check for expire on wall hit
    return true;
}