#ifndef PROJECTILE_ZONE_H
#define PROJECTILE_ZONE_H

#include "weapon/damageZone.h"

class ProjectileZone : public DamageZone {
private:
    int ricochet = 0;
    bool hasHit = false;

public:
    ProjectileZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, int ricochet=0);
    ~ProjectileZone() = default;

    bool hit(Character* other) override;
    bool update(float dt) override;
};

#endif