#ifndef DAMAGE_ZONE_H
#define DAMAGE_ZONE_H

#include "util/includes.h"
#include "character/character.h"

class DamageZone {
public:
    struct Params {
        int damage = 0;
        float life = 0;
        bool friendlyDamage = false;
        bool selfDamage = false;
        std::function<void()> onExpire = [](){};
        std::function<void(Character*)> onHit = [](Character* hit){};
    };

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
    DamageZone(Character* owner, Node2D* hitbox, Params params);
    ~DamageZone();

    void hit(Character* other);
    bool update(float dt);

    // getters
    Character* getOwner() { return this->owner; }

    // setters
    void setOnHit(std::function<void(Character*)> func) { onHit = func; }
    void setOnExpire(std::function<void()> func) { onExpire = func; }
    void setOwner(Character* owner) { this->owner = owner; }
};

#endif