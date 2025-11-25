#include "weapon/weapon.h"
#include "levels/levels.h"

Weapon::Weapon(Character* owner, Node2D::Params node, DamageZone::Params params) : owner(owner) {
    damageZoneGen = [owner, node, params](const vec2& pos, const vec2& dir) {
        return new DamageZone(owner, node, params, pos, dir);
    };
}

void Weapon::attack(const vec2& pos, const vec2& dir) {
    SingleSide* side = owner->getSide();
    DamageZone* zone = damageZoneGen(pos, dir);
    side->addDamageZone(zone);
}
