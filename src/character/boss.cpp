#include "character/boss.h"
#include "character/enemy.h"
#include "weapon/contactZone.h"
#include "levels/singleSide.h"
#include "game/game.h"
#include "game/paperView.h"
#include "util/random.h"
#include <iostream>
#include "pickup/ladder.h"


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
    vulnerableState(VulnerableState::Spawning),
    vulnerableLowerProgress(0.0f),
    vulnerableRaiseProgress(0.0f),
    vulnerableLowerDuration(0.3f),
    vulnerableRaiseDuration(0.3f),
    vulnerableRaisedHeight(0.2f),
    vulnerableLoweredHeight(0.2f),
    vulnerableTime(0.0f),
    vulnerableDuration(2.0f),
    attackWaitDuration(3.0f),
    handScale(1.2f, 0.72f, 0.3f),
    timeSinceLastVulnerable(0.0f),
    minTimeBetweenVulnerable(2.0f),
    maxTimeBetweenVulnerable(5.0f),
    nextVulnerableTime(uniform(2.0f, 5.0f)),
    iframeTimer(0.0f),
    iframeDuration(0.3f),
    spawnProgress(0.0f),
    spawnDuration(6.0f)
{
    // Create hand node in the paperView scene
    // The boss node should already exist (created in showGameElements before Boss is created)
    if (paperView && paperView->getScene()) {
        Node* bossNode = paperView->getBossNode();
        if (!bossNode) {
            std::cout << "[Boss::Boss] WARNING: Boss node doesn't exist yet!" << std::endl;
            return;
        }
        
        // Get the target position (normal starting position - bossBasePosition)
        glm::vec3 bossBasePos = paperView->getBossBasePosition();
        glm::vec3 bossPlaneNormal = paperView->getBossPlaneNormal();  // Direction from paper to camera
        glm::vec3 bossHorizontalDir = paperView->getBossHorizontalDirection();
        
        // Calculate spawn start position: off-screen (to the side and further from camera)
        // Move away from camera (in direction of plane normal, toward paper) and to the side
        spawnStartPosition = bossBasePos + bossPlaneNormal * 1.5f + bossHorizontalDir * 2.5f;
        spawnTargetPosition = bossBasePos;  // Target is the normal starting position
        
        // Immediately set boss node to spawn start position
        bossNode->setPosition(spawnStartPosition);
        
        // Initialize 2D position from spawn start (will be updated during animation)
        hand2DPosition = paperView->projectToPaperPlane(spawnStartPosition);
        
        // Reset spawn progress to ensure animation starts from beginning
        spawnProgress = 0.0f;
        // Ensure state is Spawning
        vulnerableState = VulnerableState::Spawning;
        
        // Show the boss node (will be set to appropriate material in update based on state)
        if (game) {
            bossNode->setMaterial(game->getMaterial("boss_hand_flip"));  // Show spawn animation material
        }
        
        std::cout << "[Boss::Boss] Boss created, spawnStart=(" << spawnStartPosition.x << ", " << spawnStartPosition.y << ", " << spawnStartPosition.z 
                  << "), spawnTarget=(" << spawnTargetPosition.x << ", " << spawnTargetPosition.y << ", " << spawnTargetPosition.z << ")" << std::endl;
        std::cout << "[Boss::Boss] Starting spawn animation from off-screen, state=" << static_cast<int>(vulnerableState) 
                  << " (should be 0=Spawning), spawnProgress=" << spawnProgress << std::endl;
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
    Node* bossNode = getBossNode();
    
    // Debug: Print state on first few frames
    static int frameCount = 0;
    frameCount++;
    if (frameCount <= 5) {
        std::cout << "[Boss::update] Frame " << frameCount << " - state=" << static_cast<int>(vulnerableState) 
                  << " (0=Spawning, 1=None, 2=Lowering, 3=Lowered, 4=Raising, 5=Leaving), spawnProgress=" << spawnProgress << ", health=" << health << std::endl;
    }
    
    // Handle spawn animation state FIRST - before any other logic
    if (vulnerableState == VulnerableState::Spawning) {
        if (!bossNode || !paperView) {
            std::cout << "[Boss::update] WARNING: Spawning state but bossNode=" << (bossNode ? "valid" : "null") 
                      << ", paperView=" << (paperView ? "valid" : "null") << std::endl;
            return;
        }
        
        // Ensure we start from spawn start position on first frame of spawn
        // Check if this is the very first update call (spawnProgress is exactly 0.0)
        if (spawnProgress == 0.0f) {
            bossNode->setPosition(spawnStartPosition);
            std::cout << "[Boss::update] First spawn frame - setting position to spawnStart=(" 
                      << spawnStartPosition.x << ", " << spawnStartPosition.y << ", " << spawnStartPosition.z << ")" << std::endl;
            std::cout << "[Boss::update] Current boss node position=(" 
                      << bossNode->getPosition().x << ", " << bossNode->getPosition().y << ", " << bossNode->getPosition().z << ")" << std::endl;
        }
        
        spawnProgress += dt / spawnDuration;
        
        if (spawnProgress >= 1.0f) {
            // Spawn complete - transition to normal sliding
            spawnProgress = 1.0f;
            vulnerableState = VulnerableState::None;
            // Set hand to target position
            bossNode->setPosition(spawnTargetPosition);
            // Initialize 2D position from target position
            hand2DPosition = paperView->projectToPaperPlane(spawnTargetPosition);
            hand2DPosition.x = glm::clamp(hand2DPosition.x, -0.5f, 0.5f);
            hand2DPosition.y = glm::clamp(hand2DPosition.y, -0.0f, 0.45f);
            handHeight = vulnerableRaisedHeight;
            std::cout << "[Boss::update] Spawn animation complete, transitioning to normal sliding" << std::endl;
            frameCount = 0;  // Reset frame counter
        } else {
            // Spawn animation: interpolate position from start to target
            float smoothProgress = spawnProgress * spawnProgress * (3.0f - 2.0f * spawnProgress);  // Smoothstep
            glm::vec3 currentPos = glm::mix(spawnStartPosition, spawnTargetPosition, smoothProgress);
            bossNode->setPosition(currentPos);
            
            // Debug print every second during spawn
            static int spawnDebugCount = 0;
            spawnDebugCount++;
            if (spawnDebugCount % 60 == 0) {
                std::cout << "[Boss::update] Spawning: progress=" << spawnProgress << ", pos=(" 
                          << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
            }
        }
        // Update material during spawn
        if (bossNode && game) {
            bool showBoss = game->getShowBoss();
            if (!showBoss) {
                bossNode->setMaterial(game->getMaterial("empty"));
            } else {
                bossNode->setMaterial(game->getMaterial("boss_hand_flip"));
            }
        }
        
        return;  // Don't process other states during spawn
    }
    
    // Handle leaving animation state (when boss is defeated)
    if (vulnerableState == VulnerableState::Leaving) {
        if (!bossNode || !paperView) {
            std::cout << "[Boss::update] WARNING: Leaving state but bossNode=" << (bossNode ? "valid" : "null") 
                      << ", paperView=" << (paperView ? "valid" : "null") << std::endl;
            return;
        }
        
        spawnProgress += dt / spawnDuration;
        
        if (spawnProgress >= 1.0f) {
            // Leaving complete - boss should be deleted by Game
            std::cout << "[Boss::update] Leaving animation complete, boss ready to be deleted" << std::endl;
        } else {
            // Leaving animation: interpolate position from target to start (reverse of spawn)
            float smoothProgress = spawnProgress * spawnProgress * (3.0f - 2.0f * spawnProgress);  // Smoothstep
            glm::vec3 currentPos = glm::mix(spawnTargetPosition, spawnStartPosition, smoothProgress);
            bossNode->setPosition(currentPos);
            
            // Debug print every second during leaving
            static int leavingDebugCount = 0;
            leavingDebugCount++;
            if (leavingDebugCount % 60 == 0) {
                std::cout << "[Boss::update] Leaving: progress=" << spawnProgress << ", pos=(" 
                          << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
            }
        }
        // Update material during leaving
        if (bossNode && game) {
            bool showBoss = game->getShowBoss();
            if (!showBoss) {
                bossNode->setMaterial(game->getMaterial("empty"));
            } else {
                bossNode->setMaterial(game->getMaterial("boss_hand_flip"));
            }
        }
        
        return;  // Don't process other states during leaving
    }
    
    // Check if boss should start leaving (health <= 0)
    if (health <= 0 && vulnerableState != VulnerableState::Leaving && vulnerableState != VulnerableState::Spawning) {
        std::cout << "[Boss::update] Boss defeated! Starting leaving animation" << std::endl;
        
        // Call onDeath callback
        onDeath();
        
        vulnerableState = VulnerableState::Leaving;
        spawnProgress = 0.0f;  // Start leaving animation from beginning
        vulnerable = false;  // No longer vulnerable
        // Store current position as where we're leaving from (target for leaving animation)
        if (bossNode && paperView) {
            spawnTargetPosition = bossNode->getPosition();
            // Recalculate off-screen position for leaving destination
            glm::vec3 bossBasePos = paperView->getBossBasePosition();
            glm::vec3 bossPlaneNormal = paperView->getBossPlaneNormal();
            glm::vec3 bossHorizontalDir = paperView->getBossHorizontalDirection();
            spawnStartPosition = bossBasePos + bossPlaneNormal * 1.5f + bossHorizontalDir * 2.5f;
        }
        return;  // Start leaving animation next frame
    }
    
    // Calculate speed multiplier based on health lost (1.0 at full health, 1.5 at 0 health)
    float healthPercent = static_cast<float>(health) / static_cast<float>(maxHealth);
    float speedMultiplier = 1.0f + 0.5f * (1.0f - healthPercent);  // 1.0 to 1.5
    
    // Update iframe timer
    if (iframeTimer > 0.0f) {
        iframeTimer -= dt;
        if (iframeTimer < 0.0f) {
            iframeTimer = 0.0f;
        }
    }
    
    // Update boss node material based on state and action
    if (bossNode && game) {
        bool showBoss = game->getShowBoss();
        if (!showBoss) {
            bossNode->setMaterial(game->getMaterial("empty"));
        } else {
            // Set material based on vulnerable state and action
            if (vulnerableState == VulnerableState::Lowering || vulnerableState == VulnerableState::Lowered) {
                if (currentAction == LoweredAction::Spawn) {
                    bossNode->setMaterial(game->getMaterial("boss_hand_grab"));
                } else {  // Attack
                    bossNode->setMaterial(game->getMaterial("boss_hand_slam"));
                }
            } else {
                // Normal sliding state
                bossNode->setMaterial(game->getMaterial("boss_hand_hover"));
            }
        }
    }
    
    static int updateCount = 0;
    updateCount++;
    if (updateCount % 60 == 0) {  // Print every 60 frames (roughly once per second at 60fps)
        std::cout << "[Boss::update] Called, dt=" << dt << ", bossNode=" << (bossNode ? "valid" : "null") 
                  << ", health=" << health << "/" << maxHealth << ", vulnerable=" << vulnerable 
                  << ", iframeTimer=" << iframeTimer << ", state=" << static_cast<int>(vulnerableState) << std::endl;
    }
    
    // Handle vulnerable animation states
    if (vulnerableState == VulnerableState::Lowering && bossNode && paperView) {
        // Don't update sliding animation during lowering (stop sliding)
        // bossAnimationTime is NOT updated here, so sliding stops
        
        vulnerableLowerProgress += dt * speedMultiplier / vulnerableLowerDuration;
        
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
            
            // Perform the action that was decided when lowering started
            if (currentAction == LoweredAction::Attack) {
                std::cout << "[Boss::update] Lowering complete - performing Attack action" << std::endl;
                attack();
                // Note: attack() may fail silently, but we still wait and raise
            } else {  // Spawn
                std::cout << "[Boss::update] Lowering complete - performing Spawn action" << std::endl;
                spawnEnemy();
                // For spawn, immediately start raising (even if spawn failed)
                vulnerableState = VulnerableState::Raising;
                vulnerableRaiseProgress = 0.0f;
                vulnerableTime = 0.0f;
                setVulnerable(false);  // No longer vulnerable when raising
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
        // This state only occurs for attack action (spawn immediately raises)
        handHeight = vulnerableLoweredHeight;
        
        // Convert 2D position + height to 3D
        glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
        // Ensure hand stays above paper along plane normal
        currentPos = paperView->clampAbovePaperPlane(currentPos, 0.1f);
        bossNode->setPosition(currentPos);
        
        // Handle attack wait duration (scaled by speed multiplier)
        vulnerableTime += dt * speedMultiplier;
        if (vulnerableTime >= attackWaitDuration) {
            // Start raising back up after attack wait
            vulnerableState = VulnerableState::Raising;
            vulnerableRaiseProgress = 0.0f;
            vulnerableTime = 0.0f;
            setVulnerable(false);  // No longer vulnerable when raising
        }
    } else if (vulnerableState == VulnerableState::Raising && bossNode && paperView) {
        // Don't update sliding animation during raising (stop sliding)
        // bossAnimationTime is NOT updated here, so sliding stops
        
        vulnerableRaiseProgress += dt * speedMultiplier / vulnerableRaiseDuration;
        
        if (vulnerableRaiseProgress >= 1.0f) {
            // Raising complete - return to normal sliding
            vulnerableRaiseProgress = 1.0f;
            vulnerableState = VulnerableState::None;
            setVulnerable(false);
            handHeight = vulnerableRaisedHeight;
            
            // Convert 2D position + height to 3D
            glm::vec3 currentPos = paperView->paperPlaneTo3D(hand2DPosition, handHeight);
            bossNode->setPosition(currentPos);
            // Reset timer for next vulnerable period (scaled by speed multiplier)
            timeSinceLastVulnerable = 0.0f;
            nextVulnerableTime = uniform(minTimeBetweenVulnerable, maxTimeBetweenVulnerable) / speedMultiplier;
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
        // Update animation time for movement (scaled by speed multiplier)
        bossAnimationTime += dt * bossAnimationSpeed * speedMultiplier;
        
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
        
        // Randomly trigger vulnerable period while moving (scaled by speed multiplier)
        timeSinceLastVulnerable += dt * speedMultiplier;
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
    
    // Decide action early: 75% attack, 25% spawn (so we can set the correct material during lowering)
    if (uniform() < 0.75f) {
        currentAction = LoweredAction::Attack;
    } else {
        currentAction = LoweredAction::Spawn;
    }
    
    // Start lowering animation
    vulnerableState = VulnerableState::Lowering;
    vulnerableLowerProgress = 0.0f;
    setVulnerable(false);  // Not vulnerable yet, will become vulnerable when lowered
}

void Boss::onDamage(int damage) {
    std::cout << "[Boss::onDamage] Called with damage=" << damage << ", vulnerable=" << vulnerable 
              << ", currentHealth=" << health << ", iframeTimer=" << iframeTimer 
              << ", state=" << static_cast<int>(vulnerableState) << std::endl;
    
    // Don't take damage if leaving or spawning
    if (vulnerableState == VulnerableState::Leaving || vulnerableState == VulnerableState::Spawning) {
        std::cout << "[Boss::onDamage] Ignoring damage - boss is leaving or spawning" << std::endl;
        return;
    }
    
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

void Boss::onDeath() {
    game->getSide()->addPickup(new Ladder(game, game->getSide(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("red"), .position={0.0, 0.0}, .scale={1.0, 1.0} }, 0.5f));
}

bool Boss::shouldBeDeleted() const {
    // Boss should be deleted when leaving animation is complete
    return (vulnerableState == VulnerableState::Leaving && spawnProgress >= 1.0f);
}

void Boss::spawnEnemy() {
    std::cout << "[Boss::spawnEnemy] Called" << std::endl;
    
    if (!game) {
        std::cout << "[Boss::spawnEnemy] FAILED - game is null" << std::endl;
        return;
    }
    
    // Get current side from game
    SingleSide* currentSide = game->getSide();
    if (!currentSide) {
        std::cout << "[Boss::spawnEnemy] FAILED - currentSide is null" << std::endl;
        return;
    }
    
    // Get biome from current side
    std::string biome = currentSide->getBiome();
    
    // Get enemy list for this biome
    auto biomeIt = Enemy::enemyBiomes.find(biome);
    if (biomeIt == Enemy::enemyBiomes.end() || biomeIt->second.empty()) {
        std::cout << "[Boss::spawnEnemy] FAILED - No enemies found for biome: " << biome << std::endl;
        return;
    }
    
    const auto& biomeEnemies = biomeIt->second;
    
    // Select a random enemy from the biome (simple random selection)
    int randomIndex = randint(0, static_cast<int>(biomeEnemies.size()) - 1);
    std::string selectedEnemyType = biomeEnemies[randomIndex].first;
    
    // Check if enemy template exists
    auto templateIt = Enemy::templates.find(selectedEnemyType);
    if (templateIt == Enemy::templates.end()) {
        std::cout << "[Boss::spawnEnemy] FAILED - Enemy template not found: " << selectedEnemyType << std::endl;
        return;
    }
    
    // Get 2D position for spawn
    vec2 spawnPos = get2DPosition();
    
    // Spawn the enemy
    Enemy* enemy = templateIt->second(spawnPos, currentSide);
    if (enemy) {
        currentSide->addEnemy(enemy);
        std::cout << "[Boss::spawnEnemy] SUCCESS - Spawned " << selectedEnemyType << " at (" << spawnPos.x << ", " << spawnPos.y << ")" << std::endl;
    } else {
        std::cout << "[Boss::spawnEnemy] FAILED - Enemy creation returned null for: " << selectedEnemyType << std::endl;
    }
}

vec2 Boss::get2DPosition() const {
    if (!paperView) return vec2(0.0f, 0.0f);
    
    Node* bossNode = getBossNode();
    if (!bossNode) return vec2(0.0f, 0.0f);
    
    glm::vec3 bossPos3D = bossNode->getPosition();
    // Project 3D position onto paper plane and get 2D coordinates relative to paper center
    return paperView->projectToPaperPlane(bossPos3D) * vec2{12.0f, 9.0f} + vec2{0.0f, -1.2f};
}

void Boss::attack() {
    std::cout << "[Boss::attack] Called" << std::endl;
    
    if (!game) {
        std::cout << "[Boss::attack] FAILED - game is null" << std::endl;
        return;
    }
    
    // Get current side from game
    SingleSide* currentSide = game->getSide();
    if (!currentSide) {
        std::cout << "[Boss::attack] FAILED - currentSide is null" << std::endl;
        return;
    }
    
    // Get an enemy from the side to use as owner reference (boss attacks are from enemy team)
    // If no enemies exist, spawn one first so we can use it for the attack
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
    
    // If no enemy exists, spawn one first so we can use it for the attack
    if (!ownerEnemy) {
        std::cout << "[Boss::attack] No enemy available, spawning one first for attack owner reference" << std::endl;
        spawnEnemy();
        // Try to find the enemy we just spawned
        enemies = currentSide->getEnemies();
        if (!enemies.empty()) {
            for (Enemy* enemy : enemies) {
                if (enemy && !enemy->isDead()) {
                    ownerEnemy = enemy;
                    break;
                }
            }
        }
        
        // If still no enemy (spawn failed), we can't create the attack
        if (!ownerEnemy) {
            std::cout << "[Boss::attack] FAILED - Could not spawn enemy for owner reference, skipping attack" << std::endl;
            return;
        }
    }
    
    // Get 2D position for damage zone
    vec2 attackPos = get2DPosition();
    
    // Create damage zone parameters
    DamageZone::Params params;
    params.damage = 1;  // Damage amount
    params.life = 0.5f;  // How long the damage zone lasts
    params.speed = 0.0f;  // Stationary
    params.radius = 2.0f;  // Radius for boss attack
    params.friendlyDamage = false;  // Should damage player (enemy team)
    params.selfDamage = false;
    
    // Create node parameters
    Node2D::Params nodeParams;
    nodeParams.mesh = game->getMesh("quad");
    nodeParams.material = game->getMaterial("empty");
    nodeParams.scale = vec2(params.radius * 2.0f, params.radius * 2.0f);  // Scale to match radius
    nodeParams.collider = currentSide->getCollider("quad");
    nodeParams.colliderScale = vec2(1.0f, 1.0f);
    
    // Create contact zone (stationary damage zone)
    ContactZone* damageZone = new ContactZone(ownerEnemy, nodeParams, params, attackPos);
    
    // Add to current side
    currentSide->addDamageZone(damageZone);
    
    std::cout << "[Boss::attack] SUCCESS - Created damage zone at (" << attackPos.x << ", " << attackPos.y << ")" << std::endl;
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