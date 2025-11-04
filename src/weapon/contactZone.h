#ifndef CONTACT_ZONE_H
#define CONTACT_ZONE_H

#include "weapon/damageZone.h"

class ContactZone : public DamageZone {
public:
    ContactZone();
    ~ContactZone();

    void update(float dt) override;
};

#endif