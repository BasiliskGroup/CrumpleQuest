#ifndef DAMAGE_ZONE_H
#define DAMAGE_ZONE_H

#include "util/includes.h"
#include "character/character.h"

class DamageZone : public Node2D {
public:
    struct Params {
        int damage = 0;
        float life = 0;
        float speed = 0;
        float radius = 1;
        bool friendlyDamage = false;
        bool selfDamage = false;
        std::function<void()> onExpire = [](){};
        std::function<void(Character*)> onHit = [](Character* hit){};
    };

protected:
    Character* owner;

    int damage;
    float life;
    vec2 vel;
    float radius;

    bool friendlyDamage = false;
    bool selfDamage = false;

    std::function<void()> onExpire;
    std::function<void(Character*)> onHit;

public:
    DamageZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir);
    virtual ~DamageZone() = default;

    bool virtual hit(Character* other);
    bool virtual update(float dt);
    
    Character* getOwner() const { return owner; }

    // getters
    Character* getOwner() { return this->owner; }
    float getRadius() { return radius; }
    int getDamage() const { return damage; }
    bool getFriendlyDamage() const { return friendlyDamage; }

    // setters
    void setOnHit(std::function<void(Character*)> func) { onHit = func; }
    void setOnExpire(std::function<void()> func) { onExpire = func; }
    void setOwner(Character* owner) { this->owner = owner; }
};

#endif