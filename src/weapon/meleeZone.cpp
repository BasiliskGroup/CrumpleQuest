#include "weapon/meleeZone.h"

MeleeZone::MeleeZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, float knockback) :
    DamageZone(owner, node, params, pos, dir),
    knockback(knockback)
{}

bool MeleeZone::update(float dt) {
    if (DamageZone::update(dt) == false) return false;

    // check for expire on wall hit
    return true;
}

bool MeleeZone::hit(Character* other) {
    if (DamageZone::hit(other) == false) return false;

    vec2 dir = other->getPosition() - getPosition();
    if (glm::length2(dir) < 1e-6f) return true;
    dir = glm::normalize(dir);

    // apply knockback
    other->setVelocity(other->getVelocity() + vec3{ dir * knockback, 0 });

    return true;
}