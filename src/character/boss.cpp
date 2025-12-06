#include "character/boss.h"
#include "character/enemy.h"
#include "weapon/contactZone.h"
#include "levels/singleSide.h"
#include "game/game.h"
#include "game/paperView.h"
#include "util/random.h"
#include <iostream>

Boss::Boss(Game* game, PaperView* paperView) : 
    game(game),
    paperView(paperView),
    health(100),
    maxHealth(100),
    stage("spawn"),
    vulnerable(false),
    hitboxRadius(0.5f),
    hitboxPosition(0.0f, 0.0f),
    handRaisedOffset(0.0f),
    handBaseY(0.0f),
    vulnerableState(VulnerableState::None),
    vulnerableLowerProgress(0.0f),
    vulnerableRaiseProgress(0.0f),
    vulnerableLowerDuration(0.3f),
    vulnerableRaiseDuration(0.3f),
    vulnerableRaisedPosition(0.0f),
    vulnerableLoweredPosition(0.0f),
    vulnerableTime(0.0f),
    vulnerableDuration(2.0f),
    handScale(1.2f, 0.72f, 0.3f),
    timeSinceLastVulnerable(0.0f),
    minTimeBetweenVulnerable(2.0f),
    maxTimeBetweenVulnerable(5.0f),
    nextVulnerableTime(uniform(2.0f, 5.0f)),
    currentBasePosition(0.0f)
{
    // Create hand node in the paperView scene
    // The boss node will be created later in showGameElements, so we'll initialize hand later
    if (paperView && paperView->getScene()) {
        // Get initial position from boss node if it exists, otherwise use default
        Node* bossNode = paperView->getBossNode();
        glm::vec3 bossPos = {0.0f, 0.1386f, 0.544f};  // Default paper position
        if (bossNode) {
            bossPos = bossNode->getPosition();
            currentBasePosition = bossPos;  // Initialize base position
            std::cout << "[Boss::Boss] Boss node exists, position=(" << bossPos.x << ", " << bossPos.y << ", " << bossPos.z << ")" << std::endl;
        } else {
            // If boss node doesn't exist yet, initialize with PaperView's base position
            currentBasePosition = paperView->getBossBasePosition();
            std::cout << "[Boss::Boss] Boss node doesn't exist yet, using PaperView base position=(" 
                      << currentBasePosition.x << ", " << currentBasePosition.y << ", " << currentBasePosition.z << ")" << std::endl;
        }
        handBaseY = bossPos.y;
        std::cout << "[Boss::Boss] Boss created, vulnerable=" << vulnerable << std::endl;
    }
}

Boss::~Boss() {
    // Boss node is managed by PaperView, no cleanup needed
}

Node* Boss::getBossNode() const {
    if (paperView) {
        return paperView->getBossNode();
    }
    return nullptr;
}

void Boss::setVulnerable(bool vulnerable) {
    this->vulnerable = vulnerable;
    // Hand will be raised/lowered in update() based on vulnerable state
}

void Boss::update(float dt) {
    // Update boss node material based on showBoss flag
    Node* bossNode = getBossNode();
    if (bossNode && game) {
        bool showBoss = game->getShowBoss();
        Material* material = showBoss ? game->getMaterial("boss_hand_hover") : game->getMaterial("empty");
        bossNode->setMaterial(material);
    }
    
    static int updateCount = 0;
    updateCount++;
    if (updateCount % 60 == 0) {  // Print every 60 frames (roughly once per second at 60fps)
        std::cout << "[Boss::update] Called, dt=" << dt << ", bossNode=" << (bossNode ? "valid" : "null") << std::endl;
    }
    
    // Handle vulnerable animation states
    if (vulnerableState == VulnerableState::Lowering && bossNode && paperView) {
        // Don't update sliding animation during lowering (stop sliding)
        // bossAnimationTime is NOT updated here, so sliding stops
        
        vulnerableLowerProgress += dt / vulnerableLowerDuration;
        
        if (vulnerableLowerProgress >= 1.0f) {
            // Lowering complete - now lowered and vulnerable
            vulnerableLowerProgress = 1.0f;
            vulnerableState = VulnerableState::Lowered;
            vulnerableTime = 0.0f;  // Reset vulnerable timer
            // Set to lowered position (clamp to plane height, preserve distance)
            glm::vec3 currentPos = clampToPlaneHeight(vulnerableLoweredPosition, true);
            bossNode->setPosition(currentPos);
            handBaseY = currentPos.y;
            setVulnerable(true);
            // Randomly spawn an enemy or attack when the hand lowers
            if (uniform() < 0.5f) {
                spawnEnemy();
            } else {
                attack();
            }
        } else {
            // Lowering animation: interpolate from raised to lowered position
            float smoothProgress = vulnerableLowerProgress * vulnerableLowerProgress * (3.0f - 2.0f * vulnerableLowerProgress);  // Smoothstep
            
            glm::vec3 currentPos = glm::mix(vulnerableRaisedPosition, vulnerableLoweredPosition, smoothProgress);
            
            // Clamp to plane height to keep hand between page and camera (preserve distance during animation)
            currentPos = clampToPlaneHeight(currentPos, true);
            
            bossNode->setPosition(currentPos);
            handBaseY = currentPos.y;
        }
    } else if (vulnerableState == VulnerableState::Lowered && bossNode && paperView) {
        // Stay in lowered position, no sliding (clamp to plane height, preserve distance)
        glm::vec3 currentPos = clampToPlaneHeight(vulnerableLoweredPosition, true);
        bossNode->setPosition(currentPos);
        handBaseY = currentPos.y;
        
        // Handle vulnerability recovery
        vulnerableTime += dt;
        if (vulnerableTime >= vulnerableDuration) {
            // Start raising back up
            vulnerableState = VulnerableState::Raising;
            vulnerableRaiseProgress = 0.0f;
            vulnerableTime = 0.0f;
        }
    } else if (vulnerableState == VulnerableState::Raising && bossNode && paperView) {
        // Don't update sliding animation during raising (stop sliding)
        // bossAnimationTime is NOT updated here, so sliding stops
        
        vulnerableRaiseProgress += dt / vulnerableRaiseDuration;
        
        if (vulnerableRaiseProgress >= 1.0f) {
            // Raising complete - return to normal sliding
            vulnerableRaiseProgress = 1.0f;
            vulnerableState = VulnerableState::None;
            setVulnerable(false);
            // Return to raised position (clamp to plane height, preserve distance)
            glm::vec3 currentPos = clampToPlaneHeight(vulnerableRaisedPosition, true);
            bossNode->setPosition(currentPos);
            handBaseY = currentPos.y;
            // Reset timer for next vulnerable period
            timeSinceLastVulnerable = 0.0f;
            nextVulnerableTime = uniform(minTimeBetweenVulnerable, maxTimeBetweenVulnerable);
        } else {
            // Raising animation: interpolate from lowered to raised position
            float smoothProgress = vulnerableRaiseProgress * vulnerableRaiseProgress * (3.0f - 2.0f * vulnerableRaiseProgress);  // Smoothstep
            
            glm::vec3 currentPos = glm::mix(vulnerableLoweredPosition, vulnerableRaisedPosition, smoothProgress);
            
            // Clamp to plane height to keep hand between page and camera (preserve distance during animation)
            currentPos = clampToPlaneHeight(currentPos, true);
            
            bossNode->setPosition(currentPos);
            handBaseY = currentPos.y;
        }
    } else if (vulnerableState == VulnerableState::None && bossNode && paperView) {
        // Normal random movement around the plane (only when not slapping AND not vulnerable)
        // Update animation time for movement
        bossAnimationTime += dt * bossAnimationSpeed;
        
        // Get base position from PaperView (top of screen)
        glm::vec3 bossBasePosition = paperView->getBossBasePosition();
        // Always use PaperView's base position to ensure we're at the top
        currentBasePosition = bossBasePosition;
        glm::vec3 bossHorizontalDirection = paperView->getBossHorizontalDirection();
        glm::vec3 bossVerticalDirection = paperView->getBossVerticalDirection();
        
        // Use multiple sine/cosine waves with different frequencies and phases to create random-looking circular movement
        // Different frequencies make the movement less predictable
        float t1 = bossAnimationTime * 0.7f;  // Slower horizontal component
        float t2 = bossAnimationTime * 0.9f + 1.5f;  // Slightly faster vertical component with phase offset
        float t3 = bossAnimationTime * 1.3f + 2.1f;  // Even faster component for more randomness
        
        // Combine multiple waves for more complex, random-looking motion
        float horizontalOffset = (glm::sin(t1) * 0.4f + glm::cos(t3) * 0.2f) * 0.6f;
        float verticalOffset = (glm::cos(t2) * 0.4f + glm::sin(t3) * 0.2f) * 0.6f;
        
        // Apply offsets in both horizontal and vertical directions on the plane
        glm::vec3 desiredPos = bossBasePosition + bossHorizontalDirection * horizontalOffset + bossVerticalDirection * verticalOffset;
        
        // Clamp to maintain constant height above the plane
        glm::vec3 bossPos = clampToPlaneHeight(desiredPos);
        
        // Store base Y position
        handBaseY = bossPos.y;
        
        bossNode->setPosition(bossPos);
        
        static int slidePrintCount = 0;
        slidePrintCount++;
        if (slidePrintCount % 60 == 0) {  // Print every 60 frames
            std::cout << "[Boss::update] Random movement - basePos=(" << bossBasePosition.x << ", " << bossBasePosition.y << ", " << bossBasePosition.z 
                      << "), horizontalOffset=" << horizontalOffset << ", verticalOffset=" << verticalOffset 
                      << ", finalPos=(" << bossPos.x << ", " << bossPos.y << ", " << bossPos.z << ")" << std::endl;
        }
        
        // Randomly trigger vulnerable period while moving
        timeSinceLastVulnerable += dt;
        if (timeSinceLastVulnerable >= nextVulnerableTime) {
            std::cout << "[Boss::update] Triggering vulnerable period!" << std::endl;
            startVulnerable();
        }
    } else {
        static int elseCount = 0;
        elseCount++;
        if (elseCount % 60 == 0) {
            std::cout << "[Boss::update] Not in sliding branch - vulnerableState=" << static_cast<int>(vulnerableState)
                      << ", bossNode=" << (bossNode ? "valid" : "null") 
                      << ", paperView=" << (paperView ? "valid" : "null") << std::endl;
        }
    }
    
    // Update hand scale (always use the longer scale)
    if (bossNode) {
        bossNode->setScale(handScale);
    }
    
    // Note: vulnerable animation is handled in the vulnerable state branches above
}

void Boss::startVulnerable() {
    if (vulnerableState != VulnerableState::None) return;  // Can't start vulnerable if already in vulnerable state
    
    Node* bossNode = getBossNode();
    if (!bossNode || !paperView) return;
    
    // Get current position (this is the raised position)
    glm::vec3 currentPos = bossNode->getPosition();
    vulnerableRaisedPosition = currentPos;
    
    // Get paper position and plane normal to calculate lowered position
    glm::vec3 paperPos = paperView->getPaperPosition();
    glm::vec3 bossPlaneNormal = paperView->getBossPlaneNormal();  // Direction from paper to camera (outward normal)
    
    // Calculate target position (move down toward paper along plane normal)
    // Lower the hand toward the paper, but not below it
    float verticalDistance = 0.3f;  // Distance to lower toward paper along plane normal
    
    // Calculate position relative to paper
    glm::vec3 relativePos = vulnerableRaisedPosition - paperPos;
    float distanceAlongNormal = glm::dot(relativePos, bossPlaneNormal);
    
    // Lower by reducing distance along normal, but ensure minimum distance
    float minDistance = 0.01f;  // Minimum distance from paper
    float targetDistance = glm::max(minDistance, distanceAlongNormal - verticalDistance);
    
    // Project raised position onto plane, then add back the target distance
    glm::vec3 onPlane = vulnerableRaisedPosition - bossPlaneNormal * distanceAlongNormal;
    vulnerableLoweredPosition = onPlane + bossPlaneNormal * targetDistance;
    
    // Start lowering animation
    vulnerableState = VulnerableState::Lowering;
    vulnerableLowerProgress = 0.0f;
    setVulnerable(false);  // Not vulnerable yet, will become vulnerable when lowered
}

void Boss::onDamage(int damage) {
    if (!vulnerable || health <= 0) return;
    
    health -= damage;
    if (health < 0) health = 0;
    
    // TODO: Add damage sound, effects, etc.
}

void Boss::spawnEnemy() {
    if (!game) return;
    
    // Get current side from game
    SingleSide* currentSide = game->getSide();
    if (!currentSide) return;
    
    // Get biome from current side
    std::string biome = currentSide->getBiome();
    
    // Get enemy list for this biome
    auto biomeIt = Enemy::enemyBiomes.find(biome);
    if (biomeIt == Enemy::enemyBiomes.end() || biomeIt->second.empty()) {
        std::cout << "[Boss::spawnEnemy] No enemies found for biome: " << biome << std::endl;
        return;
    }
    
    const auto& biomeEnemies = biomeIt->second;
    
    // Select a random enemy from the biome (simple random selection)
    int randomIndex = randint(0, static_cast<int>(biomeEnemies.size()) - 1);
    std::string selectedEnemyType = biomeEnemies[randomIndex].first;
    
    // Check if enemy template exists
    auto templateIt = Enemy::templates.find(selectedEnemyType);
    if (templateIt == Enemy::templates.end()) {
        std::cout << "[Boss::spawnEnemy] Enemy template not found: " << selectedEnemyType << std::endl;
        return;
    }
    
    // Get 2D position for spawn
    vec2 spawnPos = get2DPosition();
    
    // Spawn the enemy
    Enemy* enemy = templateIt->second(spawnPos, currentSide);
    if (enemy) {
        currentSide->addEnemy(enemy);
        std::cout << "[Boss::spawnEnemy] Spawned " << selectedEnemyType << " at (" << spawnPos.x << ", " << spawnPos.y << ")" << std::endl;
    } else {
        std::cout << "[Boss::spawnEnemy] Failed to create enemy: " << selectedEnemyType << std::endl;
    }
}

vec2 Boss::get2DPosition() const {
    if (!paperView) return vec2(0.0f, 0.0f);
    
    Node* bossNode = getBossNode();
    if (!bossNode) return vec2(0.0f, 0.0f);
    
    glm::vec3 bossPos3D = bossNode->getPosition();
    // Project 3D position onto paper plane and get 2D coordinates relative to paper center
    return paperView->projectToPaperPlane(bossPos3D) * vec2{12.0f, 9.0f};
}

void Boss::attack() {
    if (!game) return;
    
    // Get current side from game
    SingleSide* currentSide = game->getSide();
    if (!currentSide) return;
    
    // Get an enemy from the side to use as owner reference (boss attacks are from enemy team)
    // If no enemies exist, we can't create a damage zone with proper team checking
    auto& enemies = currentSide->getEnemies();
    Enemy* ownerEnemy = nullptr;
    if (!enemies.empty()) {
        // Find first alive enemy
        for (Enemy* enemy : enemies) {
            if (enemy && !enemy->isDead()) {
                ownerEnemy = enemy;
                break;
            }
        }
    }
    
    // If no enemy exists, create a temporary one just for the damage zone
    // This is a workaround since Boss is not a Character
    if (!ownerEnemy) {
        // We can't easily create a dummy enemy, so we'll skip the attack
        // In a real implementation, you might want to create a minimal Character
        std::cout << "[Boss::attack] No enemy available as owner reference, skipping attack" << std::endl;
        return;
    }
    
    // Get 2D position for damage zone
    vec2 attackPos = get2DPosition();
    
    // Create damage zone parameters
    DamageZone::Params params;
    params.damage = 1;  // Damage amount
    params.life = 0.5f;  // How long the damage zone lasts
    params.speed = 0.0f;  // Stationary
    params.radius = 3.0f;  // Radius as requested
    params.friendlyDamage = false;  // Should damage player (enemy team)
    params.selfDamage = false;
    
    // Create node parameters
    Node2D::Params nodeParams;
    nodeParams.mesh = game->getMesh("quad");
    nodeParams.material = game->getMaterial("circle");
    nodeParams.scale = vec2(params.radius * 2.0f, params.radius * 2.0f);  // Scale to match radius
    nodeParams.collider = currentSide->getCollider("quad");
    nodeParams.colliderScale = vec2(1.0f, 1.0f);
    
    // Create contact zone (stationary damage zone)
    ContactZone* damageZone = new ContactZone(ownerEnemy, nodeParams, params, attackPos);
    
    // Add to current side
    currentSide->addDamageZone(damageZone);
    
    std::cout << "[Boss::attack] Created damage zone at (" << attackPos.x << ", " << attackPos.y << ")" << std::endl;
}

glm::vec3 Boss::clampToPlaneHeight(const glm::vec3& position, bool preserveDistance) const {
    if (!paperView) return position;
    
    // Get plane information
    glm::vec3 paperPos = paperView->getPaperPosition();
    glm::vec3 bossPlaneNormal = paperView->getBossPlaneNormal();  // Returns direction from paper to camera (outward normal)
    glm::vec3 bossBasePosition = paperView->getBossBasePosition();
    
    // Calculate the distance from base position to paper along the plane normal
    glm::vec3 baseRelative = bossBasePosition - paperPos;
    float baseDistance = glm::dot(baseRelative, bossPlaneNormal);
    
    // Project the input position onto the plane (remove component along normal)
    glm::vec3 relativePos = position - paperPos;
    float distanceAlongNormal = glm::dot(relativePos, bossPlaneNormal);
    glm::vec3 onPlane = position - bossPlaneNormal * distanceAlongNormal;
    
    // Determine which distance to use
    float targetDistance;
    if (preserveDistance) {
        // Preserve the distance along normal (for vulnerable states)
        float minDistance = 0.01f;  // Minimum distance from paper
        targetDistance = glm::max(minDistance, distanceAlongNormal);
    } else {
        // Use base distance (for normal movement)
        targetDistance = baseDistance;
    }
    
    // Add back the desired distance along the normal
    glm::vec3 clampedPos = onPlane + bossPlaneNormal * targetDistance;
    
    // Safety check: ensure Y position never falls below paper
    clampedPos.y = glm::max(clampedPos.y, paperPos.y + 0.01f);
    
    return clampedPos;
}