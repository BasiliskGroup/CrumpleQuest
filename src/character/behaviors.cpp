#include "character/behaviors.h"
#include "character/enemy.h"
#include "character/character.h"
#include "levels/paperMesh.h"

// Helper function to check if destination is reached
static bool isDestinationReached(Enemy* enemy, const std::vector<vec2>& path) {
    std::optional<vec2> currentDestination = enemy->getCustomDestination();
    if (!currentDestination.has_value()) {
        return false; // No destination set
    }
    
    vec2 enemyPos = enemy->getPosition();
    vec2 dest = currentDestination.value();
    float distToDest = glm::length(enemyPos - dest);
    float finishRadius = enemy->getFinishRadius();
    
    // Consider destination reached if:
    // 1. We're within finishRadius of the destination, OR
    // 2. Path is empty AND we're close to the destination (within 2x finishRadius for safety)
    if (distToDest < finishRadius) {
        return true;
    } else if (path.empty()) {
        // Path is empty - check if we're close enough to consider it reached
        if (distToDest < finishRadius * 2.0f) {
            return true; // Close enough
        } else {
            // Path is empty but we're far from destination - pathfinding may have failed
            // Consider it "reached" so we pick a new destination to avoid getting stuck
            return true;
        }
    }
    
    return false;
}

// Helper function to clean up path waypoints and set movement direction
static void updatePathFollowing(Enemy* enemy, const vec2& fallbackDir = vec2(0, 0)) {
    std::vector<vec2>& path = enemy->getPath();
    vec2 enemyPos = enemy->getPosition();
    float finishRadius = enemy->getFinishRadius();
    
    // Remove waypoints we've already passed
    while (path.size() > 0 && glm::length2(path[0] - enemyPos) < finishRadius * finishRadius) {
        path.erase(path.begin());
    }
    
    // Set movement direction towards next waypoint
    if (path.size() > 0) {
        vec2 direction = path[0] - enemyPos;
        enemy->getMoveDir() = direction;
    } else {
        enemy->getMoveDir() = fallbackDir;
    }
}

// Helper function to update destination-based wandering behavior
// destinationSelector: function that takes (paperMesh, enemyPos, playerPos) and returns a target position
static void updateDestinationWandering(
    Enemy* enemy, 
    PaperMesh* paperMesh, 
    const vec2& playerPos,
    std::function<vec2(PaperMesh*, const vec2&, const vec2&)> destinationSelector,
    const vec2& fallbackDir = vec2(0, 0)
) {
    if (!paperMesh) {
        enemy->getMoveDir() = fallbackDir;
        enemy->clearCustomDestination();
        return;
    }
    
    std::vector<vec2>& path = enemy->getPath();
    
    // Check if we need a new destination
    bool needsNewDestination = false;
    if (!enemy->getCustomDestination().has_value()) {
        needsNewDestination = true;
    } else if (isDestinationReached(enemy, path)) {
        needsNewDestination = true;
    }
    
    // Pick a new destination if needed
    if (needsNewDestination) {
        enemy->clearCustomDestination();
        vec2 enemyPos = enemy->getPosition();
        vec2 targetPos = destinationSelector(paperMesh, enemyPos, playerPos);
        enemy->setCustomDestination(targetPos);
    }
    
    // Update path following
    updatePathFollowing(enemy, fallbackDir);
}

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
        enemy->clearCustomDestination();
        return;
    }
    
    // Get the paper mesh for pathfinding
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    
    // Calculate fallback direction (directly away from player)
    vec2 fallbackDir = vec2(0, 0);
    if (distance > 1e-6f) {
        fallbackDir = -glm::normalize(toPlayer);
    }
    
    // Destination selector: pick furthest of 3 random positions from player
    auto destinationSelector = [](PaperMesh* mesh, const vec2& enemyPos, const vec2& playerPos) -> vec2 {
        // Generate 3 random non-obstacle positions
        std::vector<vec2> candidatePositions;
        for (int i = 0; i < 3; ++i) {
            vec2 candidate = mesh->getRandomNonObstaclePosition();
            candidatePositions.push_back(candidate);
        }
        
        // Find the furthest position from the player
        vec2 targetPos = candidatePositions[0];
        float maxDistance = glm::length(candidatePositions[0] - playerPos);
        
        for (size_t i = 1; i < candidatePositions.size(); ++i) {
            float distToPlayer = glm::length(candidatePositions[i] - playerPos);
            if (distToPlayer > maxDistance) {
                maxDistance = distToPlayer;
                targetPos = candidatePositions[i];
            }
        }
        
        return targetPos;
    };
    
    // Update destination-based wandering
    updateDestinationWandering(enemy, paperMesh, playerPos, destinationSelector, fallbackDir);
}

void IdleBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    enemy->getMoveDir() = vec2(0, 0);
    enemy->getPath().clear();
    enemy->clearCustomDestination();
}

void StationaryBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    enemy->getMoveDir() = vec2(0, 0);
    enemy->getPath().clear();
    enemy->clearCustomDestination();
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

void WanderBehavior::update(Enemy* enemy, const vec2& playerPos, float dt) {
    // Get the paper mesh for pathfinding
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    
    // Destination selector: pick a random position, ensuring it's not too close to player
    auto destinationSelector = [](PaperMesh* mesh, const vec2& enemyPos, const vec2& playerPos) -> vec2 {
        // Get a random non-obstacle position
        vec2 targetPos = mesh->getRandomNonObstaclePosition();
        
        // Ensure we're not targeting the player's position
        float distToPlayer = glm::length(targetPos - playerPos);
        if (distToPlayer < 1.0f) {
            // If random position is too close to player, try again
            targetPos = mesh->getRandomNonObstaclePosition();
        }
        
        return targetPos;
    };
    
    // Update destination-based wandering
    updateDestinationWandering(enemy, paperMesh, playerPos, destinationSelector);
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

