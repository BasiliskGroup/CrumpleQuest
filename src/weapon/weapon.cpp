#include "weapon/weapon.h"

Weapon::Weapon(Character* owner, Node2D* hitbox, DamageZone::Params params) : owner(owner) {
    damageZoneGen = [owner, hitbox, params]() {
        return DamageZone(owner, hitbox, params);
    };
}

void Weapon::attack(const vec2& origin, const vec2& direction) {
    
}
