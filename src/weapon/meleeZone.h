#ifndef MELEE_ZONE_H
#define MELEE_ZONE_H

#include "weapon/damageZone.h"

class MeleeZone : public DamageZone {
private:
    float knockback = 0;

public:
    MeleeZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, float knockback=0);
    bool update(float dt) override;
    bool hit(Character* other) override;
};

#endif