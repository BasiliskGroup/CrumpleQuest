#include "character/behaviors.h"
#include "character/enemy.h"
#include "character/character.h"
#include "levels/paperMesh.h"

void ChasePlayerBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    // Get the paper mesh for pathfinding
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    if (!paperMesh) return;
    
    // Update path to player
    std::vector<vec2>& path = enemy->getPath();
    float padding = enemy->getRadius() * 1.2f;
    paperMesh->getPath(path, enemy->getPosition(), playerPos, padding);
    
    // Handle path following logic
    if (path.size() == 0) {
        enemy->getMoveDir() = vec2(0, 0);
        return;
    }
    
    // Remove waypoints we've already passed
    float finishRadius = enemy->getFinishRadius();
    while (path.size() > 0 && glm::length2(path[0] - enemy->getPosition()) < finishRadius * finishRadius) {
        path.erase(path.begin());
    }
    
    // Set movement direction towards next waypoint
    if (path.size() > 0) {
        enemy->getMoveDir() = path[0] - enemy->getPosition();
    } else {
        enemy->getMoveDir() = vec2(0, 0);
    }
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

void RunawayBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    vec2 enemyPos = enemy->getPosition();
    vec2 toPlayer = playerPos - enemyPos;
    float distance = glm::length(toPlayer);
    
    // Only run away if player is too close
    if (distance > minDistance) {
        enemy->getMoveDir() = vec2(0, 0);
        return;
    }
    
    // Calculate runaway direction (opposite of player)
    vec2 runawayDir = -toPlayer;
    if (distance > 1e-6f) {
        runawayDir = glm::normalize(runawayDir);
    }
    
    // Find a position to run to (far away from player)
    float runDistance = minDistance * 1.5f;
    vec2 targetPos = enemyPos + runawayDir * runDistance;
    
    // Get path away from player
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    if (!paperMesh) {
        // Fallback: just move directly away
        enemy->getMoveDir() = runawayDir;
        return;
    }
    
    std::vector<vec2>& path = enemy->getPath();
    float padding = enemy->getRadius() * 1.2f;
    paperMesh->getPath(path, enemyPos, targetPos, padding);
    
    // Handle path following
    if (path.size() == 0) {
        enemy->getMoveDir() = runawayDir;  // Fallback to direct movement
        return;
    }
    
    // Remove waypoints we've already passed
    float finishRadius = enemy->getFinishRadius();
    while (path.size() > 0 && glm::length2(path[0] - enemyPos) < finishRadius * finishRadius) {
        path.erase(path.begin());
    }
    
    // Set movement direction towards next waypoint
    if (path.size() > 0) {
        enemy->getMoveDir() = path[0] - enemyPos;
    } else {
        enemy->getMoveDir() = runawayDir;  // Fallback
    }
}

void IdleBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    enemy->getMoveDir() = vec2(0, 0);
    enemy->getPath().clear();
}

void StationaryBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    enemy->getMoveDir() = vec2(0, 0);
    enemy->getPath().clear();
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

void WanderBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    // Get the paper mesh for pathfinding
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    if (!paperMesh) {
        enemy->getMoveDir() = vec2(0, 0);
        return;
    }
    
    std::vector<vec2>& path = enemy->getPath();
    float finishRadius = enemy->getFinishRadius();
    vec2 enemyPos = enemy->getPosition();
    
    // Remove waypoints we've already passed
    while (path.size() > 0 && glm::length2(path[0] - enemyPos) < finishRadius * finishRadius) {
        path.erase(path.begin());
    }
    
    // If we have no path or reached the destination, pick a new random target
    // Only recalculate if we truly have no path (not just empty waypoints)
    if (path.empty()) {
        // Get a random non-obstacle position (NOT the player position)
        vec2 targetPos = paperMesh->getRandomNonObstaclePosition();
        std::cout << "[Wander] Generated random position: (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
        
        // Ensure we're not targeting the player's position
        float distToPlayer = glm::length(targetPos - playerPos);
        if (distToPlayer < 1.0f) {
            // If random position is too close to player, try again
            targetPos = paperMesh->getRandomNonObstaclePosition();
            std::cout << "[Wander] Regenerated random position (too close to player): (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
        }
        
        // Calculate path to the target
        float padding = enemy->getRadius() * 1.2f;
        paperMesh->getPath(path, enemyPos, targetPos, padding);
        
        // If pathfinding failed, try one more time with a different random position
        if (path.empty()) {
            targetPos = paperMesh->getRandomNonObstaclePosition();
            std::cout << "[Wander] Regenerated random position (pathfinding failed): (" << targetPos.x << ", " << targetPos.y << ")" << std::endl;
            paperMesh->getPath(path, enemyPos, targetPos, padding);
        }
        
        std::cout << "[Wander] Enemy at (" << enemyPos.x << ", " << enemyPos.y << ") targeting (" << targetPos.x << ", " << targetPos.y << "), path size: " << path.size() << std::endl;
    }
    
    // Set movement direction towards next waypoint
    if (path.size() > 0) {
        vec2 direction = path[0] - enemyPos;
        enemy->getMoveDir() = direction;
    } else {
        enemy->getMoveDir() = vec2(0, 0);
    }
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

