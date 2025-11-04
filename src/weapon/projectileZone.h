#ifndef PROJECTILE_ZONE_H
#define PROJECTILE_ZONE_H

#include "weapon/damageZone.h"

class ProjectileZone : public DamageZone {
private:
    int ricochet = 0;

public:
    ProjectileZone();
    ~ProjectileZone();

    void update(float dt) override;
};

#endif