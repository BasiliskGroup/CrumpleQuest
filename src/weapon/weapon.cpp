#include "weapon/weapon.h"

Weapon::Weapon(Character* owner, Node2D::Params node, DamageZone::Params params) : owner(owner) {
    damageZoneGen = [owner, node, params]() {
        return new DamageZone(owner, node, params);
    };
}

void Weapon::attack(const vec2& origin, const vec2& direction) {
    
}
