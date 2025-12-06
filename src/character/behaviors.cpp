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
    
    // Use moveAction to set movement direction towards next waypoint
    if (path.size() > 0) {
        enemy->getMoveAction()(path[0]);
    } else {
        // If no path, use fallback direction (or stop if fallback is zero)
        if (glm::length2(fallbackDir) > 1e-6f) {
            enemy->getMoveAction()(enemyPos + fallbackDir);
        } else {
            enemy->getMoveDir() = vec2(0, 0);
        }
    }
}

// Helper function to update destination-based wandering behavior
// destinationSelector: function that takes (paperMesh, enemyPos, playerPos) and returns a target position
static void updateDestinationWandering(
    Enemy* enemy, 
    PaperMesh* paperMesh, 
    const vec2& playerPos,
    std::function<vec2(PaperMesh*, const vec2&, const vec2&)> destinationSelector,
    const vec2& fallbackDir = vec2(0, 0),
    float refreshTime = 0.0f  // Time in seconds before refreshing destination (0 = no refresh)
) {
    if (!paperMesh) {
        if (glm::length2(fallbackDir) > 1e-6f) {
            enemy->getMoveAction()(enemy->getPosition() + fallbackDir);
        } else {
            enemy->getMoveDir() = vec2(0, 0);
        }
        enemy->clearCustomDestination();
        return;
    }
    
    std::vector<vec2>& path = enemy->getPath();
    
    // Update wander destination timer if refresh time is set
    if (refreshTime > 0.0f) {
        // Get dt from somewhere - we'll need to pass it or track it differently
        // For now, we'll track it in the behavior update function
    }
    
    // Check if we need a new destination
    bool needsNewDestination = false;
    if (!enemy->getCustomDestination().has_value()) {
        needsNewDestination = true;
        enemy->getWanderDestinationTimer() = 0.0f;  // Reset timer when setting new destination
    } else if (isDestinationReached(enemy, path)) {
        needsNewDestination = true;
        enemy->getWanderDestinationTimer() = 0.0f;  // Reset timer when destination reached
    }
    
    // Pick a new destination if needed
    if (needsNewDestination) {
        enemy->clearCustomDestination();
        vec2 enemyPos = enemy->getPosition();
        vec2 targetPos = destinationSelector(paperMesh, enemyPos, playerPos);
        enemy->setCustomDestination(targetPos);
        enemy->getWanderDestinationTimer() = 0.0f;  // Reset timer
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
    vec2 enemyPos = enemy->getPosition();
    while (path.size() > 0 && glm::length2(path[0] - enemyPos) < finishRadius * finishRadius) {
        path.erase(path.begin());
    }
    
    // Use moveAction to set movement direction towards next waypoint
    if (path.size() > 0) {
        enemy->getMoveAction()(path[0]);
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
    
    // Get the paper mesh for pathfinding
    PaperMesh* paperMesh = enemy->getPaperMeshForSide();
    
    // Calculate fallback direction (directly away from player)
    vec2 fallbackDir = vec2(0, 0);
    if (distance > 1e-6f) {
        fallbackDir = -glm::normalize(toPlayer);
    }
    
    // Destination selector: pick furthest position from player that doesn't require moving toward player
    auto destinationSelector = [](PaperMesh* mesh, const vec2& enemyPos, const vec2& playerPos) -> vec2 {
        // Calculate direction from enemy to player
        vec2 toPlayer = playerPos - enemyPos;
        float toPlayerLen = glm::length(toPlayer);
        
        // Generate candidate positions and filter out those that require moving toward player
        std::vector<vec2> validCandidates;
        for (int i = 0; i < 10; ++i) { // Try more candidates to find valid ones
            vec2 candidate = mesh->getRandomNonObstaclePosition();
            vec2 toCandidate = candidate - enemyPos;
            
            // Check if moving toward candidate would move us toward player
            // Dot product <= 0 means we're moving away or perpendicular to player
            if (toPlayerLen > 1e-6f) {
                vec2 toPlayerNormalized = toPlayer / toPlayerLen;
                float dotProduct = glm::dot(toCandidate, toPlayerNormalized);
                
                // Only accept candidates that don't move toward player
                if (dotProduct <= 0.0f) {
                    validCandidates.push_back(candidate);
                }
            } else {
                // Player is at same position, any direction is fine
                validCandidates.push_back(candidate);
            }
        }
        
        // If we found valid candidates, pick the furthest one from player
        if (!validCandidates.empty()) {
            vec2 targetPos = validCandidates[0];
            float maxDistance = glm::length(validCandidates[0] - playerPos);
            
            for (size_t i = 1; i < validCandidates.size(); ++i) {
                float distToPlayer = glm::length(validCandidates[i] - playerPos);
                if (distToPlayer > maxDistance) {
                    maxDistance = distToPlayer;
                    targetPos = validCandidates[i];
                }
            }
            
            return targetPos;
        }
        
        // No valid candidates found - try to find a position directly away from player
        // Try multiple positions in the direction away from player
        if (toPlayerLen > 1e-6f) {
            vec2 awayFromPlayer = -glm::normalize(toPlayer);
            
            // Try positions at different distances away from player
            for (float dist = 3.0f; dist <= 10.0f; dist += 2.0f) {
                vec2 candidate = enemyPos + awayFromPlayer * dist;
                // Check if this position is valid (we'll let pathfinding handle obstacles)
                // For now, just return it - pathfinding will find the closest valid path
                return candidate;
            }
        }
        
        // Ultimate fallback: return enemy position (will use fallback direction for movement)
        return enemyPos;
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
    
    // Update wander destination timer
    float& wanderTimer = enemy->getWanderDestinationTimer();
    wanderTimer += dt;
    
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
    
    // Check if we need to refresh destination after 3 seconds
    if (wanderTimer >= 3.0f && enemy->getCustomDestination().has_value()) {
        enemy->clearCustomDestination();
        wanderTimer = 0.0f;  // Reset timer - updateDestinationWandering will set new destination
    }
    
    // Update destination-based wandering (this will set a new destination if needed and reset timer)
    updateDestinationWandering(enemy, paperMesh, playerPos, destinationSelector);
    
    // Attack if we can attack (weapon ready, in range, and have line of sight)
    if (enemy->canAttackStatus() && enemy->hasLineOfSightStatus()) {
        enemy->attack(playerPos, dt);
    }
}

