#include "weapon/contactZone.h"

ContactZone::ContactZone(Character* owner, Node2D* hitbox, int damage, float life, bool friendlyDamage=false, bool selfDamage=false) :
    DamageZone(owner, hitbox, damage, life, friendlyDamage, selfDamage)
{}

bool ContactZone::update(float dt) {
    return DamageZone::update(dt);
}