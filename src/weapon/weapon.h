#ifndef WEAPON_H
#define WEAPON_H

#include "util/includes.h"
#include "weapon/damageZone.h"
#include "weapon/contactZone.h"
#include "weapon/projectileZone.h"
#include "weapon/meleeZone.h"

class Character;

class Weapon {
protected:
    Character* owner;
    std::function<DamageZone*(const vec2&, const vec2&)> damageZoneGen;
    float cooldown = 0;
    float maxCooldown;
    float range;

public:
    Weapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, float range);
    ~Weapon() = default;

    bool attack(const vec2& pos, const vec2& dir);
    void update(float dt);

    // getters
    Character* getOwner() { return owner; }
    Scene2D* getScene() { return owner->getNode()->getScene(); }
    float getCooldown() const { return cooldown; }
    bool isReady() const { return cooldown <= 0.0f; }  // Check if weapon can attack
    float getRange() const { return range; }

    // setters
    void setOwner(Character* owner) { this->owner = owner; }
};

// --------------------------
// Weapon subclasses
// --------------------------

class ContactWeapon : public Weapon {
public:
    ContactWeapon(Character* owner, Node2D::Params node, DamageZone::Params params);
};

class MeleeWeapon : public Weapon {
public: 
    MeleeWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, float knockback=0);
};

class ProjectileWeapon : public Weapon {
public: 
    ProjectileWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, int ricochet=0);
};

#endif