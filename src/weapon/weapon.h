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
    bool createDamageZone(const vec2& pos, const vec2& dir);  // Create damage zone without cooldown check (for multi-shot attacks)
    void update(float dt);

    // getters
    Character* getOwner() { return owner; }
    Scene2D* getScene() { return owner->getNode()->getScene(); }
    float getCooldown() const { return cooldown; }
    bool isReady() const { return cooldown <= 0.0f; }  // Check if weapon can attack
    float getRange() const { return range; }
    float getMaxCooldown() const { return maxCooldown; }

    // setters
    void setOwner(Character* owner) { this->owner = owner; }
    void setCooldown(float cooldownValue) { cooldown = cooldownValue; }  // Set cooldown directly (for delayed attacks)
    void setRange(float rangeValue) { range = rangeValue; }
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
private:
    std::vector<std::string> projectileMaterials;
    int materialIndex = 0;

public:
    ProjectileWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, std::vector<std::string> projectileMaterials, int ricochet=0);

    std::vector<std::string> getProjectileMaterials() const { return projectileMaterials; }
    void setProjectileMaterials(std::vector<std::string> projectileMaterials) { this->projectileMaterials = projectileMaterials; }
};

#endif