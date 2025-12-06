#include "weapon/weapon.h"
#include "levels/levels.h"

Weapon::Weapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, float range) 
    : owner(owner), maxCooldown(maxCooldown), range(range) 
{}

bool Weapon::attack(const vec2& pos, const vec2& dir) {
    if (cooldown > 0) return false;
    cooldown = maxCooldown;

    return createDamageZone(pos, dir);
}

bool Weapon::createDamageZone(const vec2& pos, const vec2& dir) {
    SingleSide* side = owner->getSide();
    DamageZone* zone = damageZoneGen(pos, dir);
    side->addDamageZone(zone);
    return true;
}

void Weapon::update(float dt) {
    cooldown -= dt;
}

// --------------------------
// Weapon subclasses    std::vector<std::string> projectileMeshes,
// --------------------------

ContactWeapon::ContactWeapon(Character* owner, Node2D::Params node, DamageZone::Params params) : Weapon(owner, node, params, 0, 1.0f) {
    damageZoneGen = [owner, node, params](const vec2& pos, const vec2& dir) {
        return new ContactZone(owner, node, params, pos);
    };
}

MeleeWeapon::MeleeWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, float knockback) : Weapon(owner, node, params, maxCooldown, 1.75f) {
    damageZoneGen = [owner, node, params, knockback](const vec2& pos, const vec2& dir) {
        return new MeleeZone(owner, node, params, pos, dir, knockback);
    };
}

ProjectileWeapon::ProjectileWeapon(Character* owner, Node2D::Params node, DamageZone::Params params, float maxCooldown, std::vector<std::string> projectileMaterials, int ricochet) 
    : Weapon(owner, node, params, maxCooldown, 100.0f), projectileMaterials(projectileMaterials) {
    damageZoneGen = [this, owner, node, params, ricochet, projectileMaterials](const vec2& pos, const vec2& dir) {
        std::string materialName = projectileMaterials[materialIndex++ % projectileMaterials.size()];
        Material* material = owner->getGame()->getMaterial(materialName);

        Node2D::Params projectileNode = node;
        projectileNode.material = material;

        return new ProjectileZone(owner, projectileNode, params, pos, dir, ricochet);
    };
}