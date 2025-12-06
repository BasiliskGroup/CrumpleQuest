#include "weapon/projectileZone.h"
#include "util/random.h"
#include "levels/levels.h"
#include <cmath>

ProjectileZone::ProjectileZone(Character* owner, Node2D::Params node, Params params, const vec2& pos, const vec2& dir, int ricochet) :
    DamageZone(owner, node, params, pos, dir),
    ricochet(ricochet)
{
    // Add random spread to the direction (spread angle in radians)
    const float spreadAngle = uniform(-0.15f, 0.15f);  // ~±8.6 degrees of spread
    float cosAngle = std::cos(spreadAngle);
    float sinAngle = std::sin(spreadAngle);
    
    // Rotate the direction vector by the spread angle
    vec2 spreadDir = {
        dir.x * cosAngle - dir.y * sinAngle,
        dir.x * sinAngle + dir.y * cosAngle
    };
    
    this->vel = spreadDir * params.speed;
    
    // Calculate rotation angle so that (-1, 0) is 0 degrees
    // Use the spread direction for rotation so the sprite faces the actual travel direction
    // Standard atan2(y, x): (1,0)=0°, (0,1)=90°, (-1,0)=180°, (0,-1)=270°
    // We want: (-1,0)=0°, (0,1)=90°, (1,0)=180°, (0,-1)=270°
    // Solution: use atan2(dir.y, -dir.x) which correctly maps:
    //   (-1,0) → atan2(0, 1) = 0° ✓
    //   (0,1) → atan2(1, 0) = 90° ✓
    //   (1,0) → atan2(0, -1) = 180° ✓
    //   (0,-1) → atan2(-1, 0) = 270° ✓
    float angle = std::atan2(spreadDir.y, spreadDir.x);
    if (angle < 0) angle += 2.0f * 3.14159265358979323846f;
    angle += 3.14159265358979323846f;
    
    // Set the rotation of the projectile node
    this->setRotation(angle);
}

bool ProjectileZone::hit(Character* other) {
    // Call parent hit method to check if damage was applied
    bool hitResult = DamageZone::hit(other);
    
    // If damage was successfully applied, mark this projectile as having hit
    if (hitResult) {
        hasHit = true;
    }
    
    return hitResult;
}

bool ProjectileZone::update(float dt) {
    // If we've hit a valid target, destroy the projectile
    if (hasHit) {
        return false;
    }
    
    if (DamageZone::update(dt) == false) return false;

    // Check if projectile entered an obstacle UVRegion
    if (owner != nullptr) {
        PaperMesh* paperMesh = owner->getPaperMeshForSide();
        if (paperMesh != nullptr) {
            vec2 projPos = getPosition();
            // Check all UVRegions for obstacle collisions
            for (const auto& uvRegion : paperMesh->regions) {
                if (uvRegion.isObstacle && uvRegion.contains(projPos)) {
                    // Destroy projectile if it's inside an obstacle
                    return false;
                }
            }
        }
    }

    // check for expire on wall hit
    return true;
}