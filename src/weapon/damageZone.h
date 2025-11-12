#ifndef DAMAGE_ZONE_H
#define DAMAGE_ZONE_H

#include "util/includes.h"
#include "character/character.h"

class DamageZone {
private:
    Character* owner;
    Node2D* hitbox;

    int damage;
    float life;

    bool friendlyDamage = false;
    bool selfDamage = false;

    std::function<void()> onExpire;
    std::function<void(Character*)> onHit;
    
public:
    DamageZone(Character* owner, Node2D* hitbox, int damage, float life, bool friendlyDamage=false, bool selfDamage=false);
    ~DamageZone();

    void hit(Character* other);
    bool update(float dt);

    // setters
    void setOnHit(std::function<void(Character*)> func) { onHit = std::move(func); }
    void setOnExpire(std::function<void()> func) { onExpire = std::move(func); }
};

#endif