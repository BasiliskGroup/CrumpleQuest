#ifndef PROJECTILE_ZONE_H
#define PROJECTILE_ZONE_H

#include "weapon/damageZone.h"

class ProjectileZone : public DamageZone {
private:
    int ricochet = 0;

public:
    ProjectileZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, int ricochet=0);
    bool update(float dt);
};

#endif