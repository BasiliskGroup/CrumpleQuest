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
    health(20),
    maxHealth(20),
    stage("spawn"),
    vulnerable(false),
    hitboxRadius(0.5f),
    hitboxPosition(0.0f, 0.0f),
    hand2DPosition(0.0f, 0.0f),
    handHeight(0.2f),
    vulnerableState(VulnerableState::None),
    vulnerableLowerProgress(0.0f),
    vulnerableRaiseProgress(0.0f),
    vulnerableLowerDuration(0.3f),
    vulnerableRaiseDuration(0.3f),
    vulnerableRaisedHeight(0.2f),
    vulnerableLoweredHeight(0.2f),
    vulnerableTime(0.0f),
    vulnerableDuration(2.0f),
    handScale(1.2f, 0.72f, 0.3f),
    timeSinceLastVulnerable(0.0f),
    minTimeBetweenVulnerable(2.0f),
    maxTimeBetweenVulnerable(5.0f),
    nextVulnerableTime(uniform(2.0f, 5.0f)),
    iframeTimer(0.0f),
    iframeDuration(0.3f)
{
    // Create hand node in the paperView scene
    // The boss node will be created later in showGameElements, so we'll initialize hand later
    if (paperView && paperView->getScene()) {
        // Get initial position from boss node if it exists, otherwise use default
        Node* bossNode = paperView->getBossNode();
        glm::vec3 bossPos = {0.0f, 0.1386f, 0.544f};  // Default paper position
        if (bossNode) {
            bossPos = bossNode->getPosition();
            // Initialize 2D position from current 3D position
            hand2DPosition = paperView->projectToPaperPlane(bossPos);
            // Clamp to bounds
            hand2DPosition.x = glm::clamp(hand2DPosition.x, -0.5f, 0.5f);
            hand2DPosition.y = glm::clamp(hand2DPosition.y, -0.4f, 0.4f);
            std::cout << "[Boss::Boss] Boss node exists, position=(" << bossPos.x << ", " << bossPos.y << ", " << bossPos.z << ")" << std::endl;
        } else {
            // If boss node doesn't exist yet, start at center
            hand2DPosition = vec2(0.0f, 0.0f);
            std::cout << "[Boss::Boss] Boss node doesn't exist yet, using center position" << std::endl;
        }
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
    // Update iframe timer
    if (iframeTimer > 0.0f) {
        iframeTimer -= dt;
        if (iframeTimer < 0.0f) {
            iframeTimer = 0.0f;
        }
    }
    
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
        std::cout << "[Boss::update] Called, dt=" << dt << ", bossNode=" << (bossNode ? "valid" : "null") 
                  << ", health=" << health << "/" << maxHealth << ", vulnerable=" << vulnerable 
                  << ", iframeTimer=" << iframeTimer << std::endl;
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
            handHeight = vulnerableLoweredHeight;
            
            // Convert 2D position + height to 3D
            glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
            // Ensure hand stays above paper along plane normal
            currentPos = paperView->clampAbovePaperPlane(currentPos, 0.1f);
            bossNode->setPosition(currentPos);
            setVulnerable(true);
            // Randomly spawn an enemy or attack when the hand lowers
            if (uniform() < 0.5f) {
                spawnEnemy();
            } else {
                attack();
            }
        } else {
            // Lowering animation: interpolate height from raised to lowered
            float smoothProgress = vulnerableLowerProgress * vulnerableLowerProgress * (3.0f - 2.0f * vulnerableLowerProgress);  // Smoothstep
            
            handHeight = glm::mix(vulnerableRaisedHeight, vulnerableLoweredHeight, smoothProgress);
            
            // Convert 2D position + height to 3D
            glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
            // Ensure hand stays above paper along plane normal during lowering animation
            currentPos = paperView->clampAbovePaperPlane(currentPos, 0.1f);
            bossNode->setPosition(currentPos);
        }
    } else if (vulnerableState == VulnerableState::Lowered && bossNode && paperView) {
        // Stay in lowered position, no sliding
        handHeight = vulnerableLoweredHeight;
        
        // Convert 2D position + height to 3D
        glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
        // Ensure hand stays above paper along plane normal
        currentPos = paperView->clampAbovePaperPlane(currentPos, 0.1f);
        bossNode->setPosition(currentPos);
        
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
            handHeight = vulnerableRaisedHeight;
            
            // Convert 2D position + height to 3D
            glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
            bossNode->setPosition(currentPos);
            // Reset timer for next vulnerable period
            timeSinceLastVulnerable = 0.0f;
            nextVulnerableTime = uniform(minTimeBetweenVulnerable, maxTimeBetweenVulnerable);
        } else {
            // Raising animation: interpolate height from lowered to raised
            float smoothProgress = vulnerableRaiseProgress * vulnerableRaiseProgress * (3.0f - 2.0f * vulnerableRaiseProgress);  // Smoothstep
            
            handHeight = glm::mix(vulnerableLoweredHeight, vulnerableRaisedHeight, smoothProgress);
            
            // Convert 2D position + height to 3D
            glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
            bossNode->setPosition(currentPos);
        }
    } else if (vulnerableState == VulnerableState::None && bossNode && paperView) {
        // Normal random movement around the plane (only when not slapping AND not vulnerable)
        // Update animation time for movement
        bossAnimationTime += dt * bossAnimationSpeed;
        
        // Use sine wave to create back-and-forth horizontal motion within bounds
        // Bounds are (-0.5, -0.4) to (0.5, 0.5)
        float horizontalOffset = glm::sin(bossAnimationTime) * 0.5f;  // ±0.5 units horizontally
        float verticalOffset = glm::cos(bossAnimationTime * 0.5f) * 0.4f;  // ±0.5 units vertically (slower)

        // Update 2D position within bounds
        hand2DPosition.x = glm::clamp(horizontalOffset, -0.5f, 0.5f);
        hand2DPosition.y = glm::clamp(verticalOffset, -0.0f, 0.45f);
        
        // Convert 2D position + height to 3D
        glm::vec3 bossPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
        bossNode->setPosition(bossPos);
        
        static int slidePrintCount = 0;
        slidePrintCount++;
        if (slidePrintCount % 60 == 0) {  // Print every 60 frames
            std::cout << "[Boss::update] Sliding - 2DPos=(" << hand2DPosition.x << ", " << hand2DPosition.y 
                      << "), height=" << handHeight << ", finalPos=(" << bossPos.x << ", " << bossPos.y << ", " << bossPos.z << ")" << std::endl;
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
    
    // Store current height as raised height
    vulnerableRaisedHeight = handHeight;
    
    // Lowered height is 0 (at paper level)
    vulnerableLoweredHeight = 0.0f;
    
    // Start lowering animation
    vulnerableState = VulnerableState::Lowering;
    vulnerableLowerProgress = 0.0f;
    setVulnerable(false);  // Not vulnerable yet, will become vulnerable when lowered
}

void Boss::onDamage(int damage) {
    std::cout << "[Boss::onDamage] Called with damage=" << damage << ", vulnerable=" << vulnerable 
              << ", currentHealth=" << health << ", iframeTimer=" << iframeTimer << std::endl;
    
    // Check iframes first
    if (iframeTimer > 0.0f) {
        std::cout << "[Boss::onDamage] Ignoring damage - iframes active (timer=" << iframeTimer << ")" << std::endl;
        return;
    }
    
    if (!vulnerable || health <= 0) {
        std::cout << "[Boss::onDamage] Ignoring damage - vulnerable=" << vulnerable << ", health=" << health << std::endl;
        return;
    }
    
    health -= damage;
    if (health < 0) health = 0;
    
    // Activate iframes after taking damage
    iframeTimer = iframeDuration;
    
    std::cout << "[Boss::onDamage] Boss took " << damage << " damage! New health: " << health << "/" << maxHealth 
              << ", iframes activated for " << iframeDuration << " seconds" << std::endl;
    
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
    return paperView->projectToPaperPlane(bossPos3D) * vec2{12.0f, 9.0f} + vec2{0.0f, 0.15f};
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