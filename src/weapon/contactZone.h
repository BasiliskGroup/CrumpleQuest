#ifndef CONTACT_ZONE_H
#define CONTACT_ZONE_H

#include "weapon/damageZone.h"

class ContactZone : public DamageZone {
public:
    ContactZone(Character* owner, Node2D::Params node, int damage, float life, bool friendlyDamage=false, bool selfDamage=false);

    bool update(float dt);
};

#endif