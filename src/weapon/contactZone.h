#ifndef CONTACT_ZONE_H
#define CONTACT_ZONE_H

#include "weapon/damageZone.h"

class ContactZone : public DamageZone {
public:
    ContactZone(Character* owner, Node2D::Params node, Params params, const vec2& pos);

    bool update(float dt) override;
};

#endif