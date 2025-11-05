#ifndef DAMAGE_ZONE_H
#define DAMAGE_ZONE_H

#include "util/includes.h"
#include "character/character.h"

auto onDamage = [](Character* hit) { hit->onDamage(5); };

class DamageZone {
private:
    Character* owner;
    std::string team;
    Node2D* hitbox;

    int damage;
    float life;

    bool friendlyDamage = false;
    bool selfDamage = false;

    // onExpire()
    // onHit(Character* hit)
    
public:
    DamageZone();
    ~DamageZone();

    virtual void update(float dt);
    void setOnExpire();
    void setOnHit();
};

#endif