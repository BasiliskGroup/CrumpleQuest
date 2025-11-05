#include "weapon/weapon.h"

Weapon::Weapon(Character* owner, DamageZone zne) : owner(owner), zone(zone) {}

void Weapon::attack(const vec2& origin, const vec2& direction) {
    
}
