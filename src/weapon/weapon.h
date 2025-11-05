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
    Character* owner;
    std::string team;

public:
    Weapon(Character* owner, DamageZone zone);
    ~Weapon() = default;

    void attack(const vec2& origin, const vec2& direction);

};

#endif