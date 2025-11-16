#ifndef WEAPON_H
#define WEAPON_H

#include "util/includes.h"
#include "weapon/damageZone.h"
#include "weapon/contactZone.h"
#include "weapon/projectileZone.h"

class Character;

class Weapon {
private:
    DamageZone zone;
    std::string team;

public:
    Weapon(DamageZone zone);
    ~Weapon() = default;

    void attack(const vec2& origin, const vec2& direction);

    // getters
    Character* getOwner() { return this->zone.getOwner(); }

    // setters
    void setOwner(Character* owner) { this->zone.setOwner(owner); }
};

#endif