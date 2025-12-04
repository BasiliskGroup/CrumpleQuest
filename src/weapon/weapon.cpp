#include "weapon/weapon.h"
#include "levels/levels.h"

Weapon::Weapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown) 
    : owner(owner), maxCooldown(maxCooldown) 
{}

bool Weapon::attack(const vec2& pos, const vec2& dir) {
    if (cooldown > 0) return false;
    cooldown = maxCooldown;

    SingleSide* side = owner->getSide();
    DamageZone* zone = damageZoneGen(pos, dir);
    side->addDamageZone(zone);

    return true;
}

void Weapon::update(float dt) {
    cooldown -= dt;
}

// --------------------------
// Weapon subclasses
// --------------------------

ContactWeapon::ContactWeapon(Character* owner, Node2D::Params node, DamageZone::Params params) : Weapon(owner, node, params, 0) {
    damageZoneGen = [owner, node, params](const vec2& pos, const vec2& dir) {
        return new ContactZone(owner, node, params, pos);
    };
}

MeleeWeapon::MeleeWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, float knockback) : Weapon(owner, node, params, maxCooldown) {
    damageZoneGen = [owner, node, params, knockback](const vec2& pos, const vec2& dir) {
        return new MeleeZone(owner, node, params, pos, dir, knockback);
    };
}

ProjectileWeapon::ProjectileWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, int ricochet) : Weapon(owner, node, params, maxCooldown) {
    damageZoneGen = [owner, node, params, ricochet](const vec2& pos, const vec2& dir) {
        return new ProjectileZone(owner, node, params, pos, dir, ricochet);
    };
}