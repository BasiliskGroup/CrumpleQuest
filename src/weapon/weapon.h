#ifndef WEAPON_H
#define WEAPON_H

#include "util/includes.h"
#include "weapon/damageZone.h"
#include "weapon/contactZone.h"
#include "weapon/projectileZone.h"

class Character;

class Weapon {
private:
    Character* owner;
    std::function<DamageZone()> damageZoneGen;

public:
    Weapon(Character* owner, Node2D* hitbox, DamageZone::Params params);
    ~Weapon() = default;

    void attack(const vec2& origin, const vec2& direction);

    // getters
    Character* getOwner() { return owner; }
    Scene2D* getScene() { return owner->getNode()->getScene(); }

    // setters
    void setOwner(Character* owner) { this->owner = owner; }
};

#endif