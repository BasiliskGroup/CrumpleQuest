#ifndef PROJECTILE_ZONE_H
#define PROJECTILE_ZONE_H

#include "weapon/damageZone.h"

class ProjectileZone : public DamageZone {
private:
    int ricochet = 0;

public:
    ProjectileZone(Character* owner, Node2D* hitbox, int damage, float life, int ricochet=0, bool friendlyDamage=false, bool selfDamage=false);

    bool update(float dt);
};

#endif