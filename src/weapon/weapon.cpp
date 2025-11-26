#include "weapon/weapon.h"
#include "levels/levels.h"

Weapon::Weapon(Character* owner, Node2D::Params node, DamageZone::Params params) : owner(owner) {
    // damageZoneGen = [owner, node, params](const vec2& pos, const vec2& dir) {
    //     return new DamageZone(owner, node, params, pos, dir);
    // };
}

void Weapon::attack(const vec2& pos, const vec2& dir) {
    SingleSide* side = owner->getSide();
    DamageZone* zone = damageZoneGen(pos, dir);
    side->addDamageZone(zone);
}

// --------------------------
// Weapon subclasses
// --------------------------

ContactWeapon::ContactWeapon(Character* owner, Node2D::Params node, DamageZone::Params params) : Weapon(owner, node, params) {
    damageZoneGen = [owner, node, params](const vec2& pos, const vec2& dir) {
        return new ContactZone(owner, node, params, pos);
    };
}

MeleeWeapon::MeleeWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float knockback) : Weapon(owner, node, params) {
    damageZoneGen = [owner, node, params, knockback](const vec2& pos, const vec2& dir) {
        return new MeleeZone(owner, node, params, pos, dir, knockback);
    };
}

ProjectileWeapon::ProjectileWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, int ricochet) : Weapon(owner, node, params) {
    damageZoneGen = [owner, node, params, ricochet](const vec2& pos, const vec2& dir) {
        return new ProjectileZone(owner, node, params, pos, dir, ricochet);
    };
}