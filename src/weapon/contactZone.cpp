#include "weapon/contactZone.h"

ContactZone::ContactZone(Character* owner, Node2D::Params node, Params params, const vec2& pos) :
    DamageZone(owner, node, params, pos, vec2())
{}

bool ContactZone::update(float dt) {
    return DamageZone::update(dt);
}