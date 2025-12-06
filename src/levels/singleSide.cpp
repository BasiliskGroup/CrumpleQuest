#include "levels/levels.h"
#include "weapon/meleeZone.h"
#include "util/random.h"
#include "pickup/pickup.h"
#include "pickup/heart.h"
#include "pickup/stapleGun.h"
#include "pickup/scissor.h"
#include "pickup/ladder.h"
#include "character/boss.h"


SingleSide::SingleSide(Game* game, std::string mesh, std::string material, vec2 playerSpawn, std::string biome, std::vector<vec2> enemySpawns, float difficulty) : 
    scene(nullptr), 
    camera(nullptr), 
    background(nullptr), 
    playerNode(nullptr), 
    weaponNode(nullptr), 
    playerSpawn(playerSpawn),
    enemySpawns(enemySpawns),
    biome(biome),
    difficulty(difficulty)
{
    scene = new Scene2D(game->getEngine());
    this->camera = new StaticCamera2D(game->getEngine());
    this->scene->setCamera(this->camera);
    this->camera->setScale(12.0f, 9.0f);
    this->scene->getSolver()->setGravity(0);

    loadResources();

    // create player node
    playerNode = SingleSide::genPlayerNode(game, this);
    weaponNode = new Node2D(playerNode, { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .scale={1, 1} });

    // create background node
    background = new Node2D(scene, { 
        .mesh=game->getMesh(mesh), 
        .material=game->getMaterial(material), 
        .scale={ 1, 1 } 
    });
    background->setLayer(-0.7);

    // transform enemy spawns
    vec2 offset = {6.0f, 4.5f};
    for (vec2& spawn : enemySpawns) {
        spawn -= offset;
    }

    for (vec2& spawn : this->enemySpawns) {
        spawn -= offset;
    }

    // add enemies based on biome - uniformly select from biome enemies
    if (!enemySpawns.empty()) {
        // Get enemy list for this biome
        auto biomeIt = Enemy::enemyBiomes.find(biome);
        if (biomeIt != Enemy::enemyBiomes.end()) {
            const auto& biomeEnemies = biomeIt->second;
            
            // Only proceed if we have valid enemies for this biome
            if (!biomeEnemies.empty()) {
                // Fill spawn points with enemies - each spawn has 50% chance
                for (const vec2& spawnPos : enemySpawns) {
                    // 50% chance to spawn an enemy at this location
                    if (uniform() < 0.8f) {
                        // Uniformly select from all enemies in biome
                        int randomIndex = randint(0, static_cast<int>(biomeEnemies.size() - 1));
                        const std::string& selectedEnemy = biomeEnemies[randomIndex].first;
                        
                        // Create and add the enemy
                        auto templateIt = Enemy::templates.find(selectedEnemy);
                        if (templateIt != Enemy::templates.end()) {
                            Enemy* enemy = templateIt->second(spawnPos, this);
                            if (enemy != nullptr) {
                                addEnemy(enemy);
                            }
                        }
                    }
                }
            }
        }
    }
}

SingleSide::SingleSide(const SingleSide& other) noexcept 
    : scene(nullptr), camera(nullptr), background(nullptr), playerNode(nullptr), weaponNode(nullptr)
{
    if (other.scene) scene = new Scene2D(*other.scene);
    if (other.camera) camera = new StaticCamera2D(*other.camera);
    enemies = other.enemies;
    damageZones = other.damageZones;
    pickups = other.pickups;
    enemySpawns = other.enemySpawns;
    difficulty = other.difficulty;

    loadResources();
    
    // find background - iterate both trees in parallel
    if (other.background) {
        auto thisIt = scene->getRoot()->begin();
        auto otherIt = other.scene->getRoot()->begin();
        auto thisEnd = scene->getRoot()->end();
        auto otherEnd = other.scene->getRoot()->end();
        
        while (otherIt != otherEnd && thisIt != thisEnd) {
            if (*otherIt == other.background) {
                background = *thisIt;
                break;
            }
            ++otherIt;
            ++thisIt;
        }
    }
    
    if (other.playerNode) {
        auto thisIt = scene->getRoot()->begin();
        auto otherIt = other.scene->getRoot()->begin();
        auto thisEnd = scene->getRoot()->end();
        auto otherEnd = other.scene->getRoot()->end();
        
        while (otherIt != otherEnd && thisIt != thisEnd) {
            if (*otherIt == other.playerNode) {
                playerNode = *thisIt;
                break;
            }
            ++otherIt;
            ++thisIt;
        }
    }

    if (other.weaponNode) {
        auto thisIt = scene->getRoot()->begin();
        auto otherIt = other.scene->getRoot()->begin();
        auto thisEnd = scene->getRoot()->end();
        auto otherEnd = other.scene->getRoot()->end();
        
        while (otherIt != otherEnd && thisIt != thisEnd) {
            if (*otherIt == other.weaponNode) {
                weaponNode = *thisIt;
                break;
            }
            ++otherIt;
            ++thisIt;
        }
    }
}

SingleSide::SingleSide(SingleSide&& other) noexcept : 
    scene(other.scene),
    camera(other.camera),
    enemies(std::move(other.enemies)),
    damageZones(std::move(other.damageZones)), 
    background(nullptr),
    playerNode(nullptr),
    playerSpawn(other.playerSpawn),
    enemySpawns(other.enemySpawns),
    weaponNode(nullptr),
    difficulty(other.difficulty)
    // TODO copy over player and weapon node
{
    other.scene = nullptr;
    other.camera = nullptr;
    // other.background = nullptr;
    // other.playerNode = nullptr;
    // other.weaponNode = nullptr;

    loadResources();
}

SingleSide::~SingleSide() {
    clear();
}

SingleSide& SingleSide::operator=(const SingleSide& other) noexcept {
    if (this == &other) return *this;
    clear();

    if (other.scene) scene = new Scene2D(*other.scene);
    if (other.camera) camera = new StaticCamera2D(*other.camera);

    enemies = other.enemies;
    damageZones = other.damageZones;
    playerSpawn = other.playerSpawn;
    enemySpawns = other.enemySpawns;
    difficulty = other.difficulty;
    return *this;
}

SingleSide& SingleSide::operator=(SingleSide&& other) noexcept {
    if (this == &other) return *this;
    clear();

    // transfer
    scene = other.scene;
    camera = other.camera;
    enemies = std::move(other.enemies);
    damageZones = std::move(other.damageZones);
    pickups = std::move(other.pickups);
    playerSpawn = other.playerSpawn;
    enemySpawns = other.enemySpawns;
    difficulty = other.difficulty;

    // clear other
    other.scene = nullptr;
    other.camera = nullptr;
    return *this;
}

void SingleSide::generateNavmesh() {
    
}

void SingleSide::update(const vec2& playerPos, float dt, Player* player) {
    // update all damageZones
    // done before enemy update to give a "summoning sickness" for a single frame
    for (int i = 0; i < damageZones.size(); i++) {
        DamageZone* zone = damageZones[i];

        bool good = zone->update(dt);
        if (good == false) {
            delete zone; zone = nullptr;
            damageZones.erase(damageZones.begin() + i--);
            continue;
        }
    }

    // Check for collisions between player damage zones and enemy damage zones
    // This happens before character collision checks
    for (int i = 0; i < damageZones.size(); i++) {
        DamageZone* playerZone = damageZones[i];
        if (!playerZone || playerZone->getOwner()->getTeam() != "Ally") continue;

        for (int j = 0; j < damageZones.size(); j++) {
            DamageZone* enemyZone = damageZones[j];
            if (!enemyZone || enemyZone->getOwner()->getTeam() != "Enemy") continue;
            if (i == j) continue; // Don't check a zone against itself

            // Check collision between the two zones
            float combinedRadius = playerZone->getRadius() + enemyZone->getRadius();
            float distSq = glm::length2(playerZone->getPosition() - enemyZone->getPosition());
            
            if (distSq <= combinedRadius * combinedRadius) {
                // Player zone hit enemy zone - remove enemy zone and play sound
                Character* enemyOwner = enemyZone->getOwner();
                std::string damageSound = enemyOwner->getDamageSound();
                
                if (!damageSound.empty()) {
                    audio::SFXPlayer::Get().Play(damageSound);
                }
                
                // Remove the enemy zone
                delete enemyZone; enemyZone = nullptr;
                damageZones.erase(damageZones.begin() + j);
                
                // Adjust indices since we removed an element
                if (j < i) i--;
                j--; // Decrement j to account for removed element
            }
        }
    }

    // Now check collisions with characters
    for (int i = 0; i < damageZones.size(); i++) {
        DamageZone* zone = damageZones[i];

        // check collision with enemies
        for (int j = 0; j < enemies.size(); j++) {
            Enemy* enemy = enemies[j];
            if (enemy->isDead()) continue; // Skip already dead enemies
            float combinedRadius = enemy->getRadius() + zone->getRadius();
            if (glm::length2(enemy->getPosition() - zone->getPosition()) > combinedRadius * combinedRadius) continue;
            zone->hit(enemy);
        }

        // check collision with player (if player exists and is on this side)
        if (player != nullptr && !player->isDead()) {
            float combinedRadius = player->getRadius() + zone->getRadius();
            float distSq = glm::length2(player->getPosition() - zone->getPosition());
            if (distSq <= combinedRadius * combinedRadius) {
                zone->hit(player);
            }
        }

        // check collision with boss (if boss exists, is vulnerable, damage zone is not friendly, and owner is not Enemy team)
        if (player != nullptr && player->getGame() != nullptr) {
            Boss* boss = player->getGame()->getBoss();
            if (boss != nullptr) {
                bool isVulnerable = boss->isVulnerable();
                bool isFriendly = zone->getFriendlyDamage();
                // Boss is immune to damage zones from Enemy team
                Character* zoneOwner = zone->getOwner();
                bool isEnemyTeam = (zoneOwner != nullptr && zoneOwner->getTeam() == "Enemy");
                
                static int bossCheckCount = 0;
                bossCheckCount++;
                if (bossCheckCount % 60 == 0) {  // Print every 60 frames
                    std::cout << "[SingleSide::update] Boss check - exists=" << (boss != nullptr) 
                              << ", vulnerable=" << isVulnerable << ", zoneFriendly=" << isFriendly 
                              << ", isEnemyTeam=" << isEnemyTeam << std::endl;
                }
                
                if (isVulnerable && !isFriendly && !isEnemyTeam) {
                    // Get boss 2D position
                    vec2 bossPos = boss->get2DPosition();
                    // Use attack radius (2.0f) for boss hitbox
                    float bossRadius = 2.0f;
                    float combinedRadius = bossRadius + zone->getRadius();
                    float distSq = glm::length2(bossPos - zone->getPosition());
                    
                    static int collisionCheckCount = 0;
                    collisionCheckCount++;
                    if (collisionCheckCount % 60 == 0) {  // Print every 60 frames
                        std::cout << "[SingleSide::update] Boss collision check - bossPos=(" << bossPos.x << ", " << bossPos.y 
                                  << "), zonePos=(" << zone->getPosition().x << ", " << zone->getPosition().y 
                                  << "), distSq=" << distSq << ", combinedRadiusSq=" << (combinedRadius * combinedRadius) << std::endl;
                    }
                    
                    if (distSq <= combinedRadius * combinedRadius) {
                        std::cout << "[SingleSide::update] BOSS HIT! Calling onDamage with " << zone->getDamage() << " damage" << std::endl;
                        // Boss takes damage from non-friendly damage zones (excluding Enemy team)
                        boss->onDamage(zone->getDamage());
                    }
                }
            }
        }
    }

    // remove all dead enemies
    for (int i = 0; i < enemies.size(); i++) {
        Enemy* enemy = enemies[i];
        if (enemy->isDead() == false) continue;

        // Remove all damage zones owned by this enemy before deleting it
        for (int j = 0; j < damageZones.size(); j++) {
            if (damageZones[j]->getOwner() == enemy) {
                delete damageZones[j];
                damageZones.erase(damageZones.begin() + j);
                j--;
            }
        }

        enemy->onDeath();
        enemies.erase(enemies.begin() + i);
        delete enemy;
        i--;
    }

    // update all enemies
    for (Enemy* enemy : enemies) {
        enemy->updateStatus(playerPos);
        enemy->move(playerPos, dt);
    }

    // update all pickups
    for (Pickup* pickup : pickups) {
        if (pickup != nullptr) {
            pickup->update(dt);
        }
    }

    // Check for collisions between player and pickups
    if (player != nullptr && !player->isDead()) {
        for (int i = 0; i < pickups.size(); i++) {
            Pickup* pickup = pickups[i];
            if (pickup == nullptr) continue;
            
            // Check collision first
            float combinedRadius = player->getRadius() + pickup->getRadius();
            float distSq = glm::length2(player->getPosition() - pickup->getPosition());
            
            if (distSq <= combinedRadius * combinedRadius) {
                // Player collided with pickup - check if it can be picked up
                if (pickup->canPickup(player)) {
                    pickup->onPickup();
                    delete pickup;
                    pickups.erase(pickups.begin() + i);
                    i--; // Adjust index after removal
                }
            }
        }
    }

    scene->update();
}

void SingleSide::clear() {
    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    for (Pickup* pickup : pickups) {
        delete pickup;
    }
    pickups.clear();
    
    walls.clear(); // will get cleaned by the scene

    delete scene; scene = nullptr;
    delete camera; camera = nullptr;
}

void SingleSide::clearWalls() {
    for (auto& wall : walls) {
        delete wall;
    }
    walls.clear();
}

void SingleSide::loadResources() {
    addCollider("quad", new Collider(scene->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));
}

void SingleSide::adoptEnemy(Enemy* enemy, SingleSide* fromSide) {
    if (enemy == nullptr || fromSide == nullptr) return;
    if (fromSide == this) return; // Already in this side
    
    Node2D* oldNode = enemy->getNode();
    if (oldNode == nullptr) return;
    
    // Save all properties from the old node
    // Properties we currently have getters for:
    vec2 position = oldNode->getPosition();
    vec2 scale = oldNode->getScale();
    vec3 velocity = oldNode->getVelocity();
    Mesh* mesh = oldNode->getMesh();
    Material* material = oldNode->getMaterial();
    Collider* collider = getCollider("quad");
    vec2 colliderScale = oldNode->getColliderScale();
    float density = oldNode->getDensity();
    float rotation = oldNode->getRotation();
    float layer = oldNode->getLayer();
    
    // Remove enemy from old side's enemies list
    auto& oldEnemies = fromSide->getEnemies();
    for (auto it = oldEnemies.begin(); it != oldEnemies.end(); ++it) {
        if (*it == enemy) {
            oldEnemies.erase(it);
            break;
        }
    }
    
    // Delete the old node from the old scene
    delete oldNode;
    
    // Create a new node in this side's scene with the same properties
    Node2D* newNode = new Node2D(scene, {
        .mesh = mesh,
        .material = material,
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .collider = collider,
        .colliderScale = colliderScale,
        .density = density
    });
    
    // Restore additional properties
    newNode->setVelocity(velocity);
    if (layer != 0.0f) { // Only set layer if it was non-zero (assuming 0 is default)
        newNode->setLayer(layer);
    }
    
    // Set manifold mask (as done in Character constructor)
    newNode->setManifoldMask(1, 1, 0);
    
    // Update enemy's node and side pointers
    // Use updateNode to also update the animator's node reference
    enemy->updateNode(newNode);
    enemy->setSide(this);
    
    // Add enemy to this side's enemies list
    addEnemy(enemy);
}

Pickup* SingleSide::adoptPickup(Pickup* pickup, SingleSide* fromSide) {
    if (pickup == nullptr || fromSide == nullptr) return nullptr;
    if (fromSide == this) return pickup; // Already in this side
    
    // Save all Node2D properties from the old pickup before deleting
    // Pickups are static objects, so we don't need to preserve velocity
    vec2 position = pickup->getPosition();
    vec2 scale = pickup->getScale();
    Mesh* mesh = pickup->getMesh();
    Material* material = pickup->getMaterial();
    float rotation = pickup->getRotation();
    float layer = pickup->getLayer();
    
    // Save pickup-specific properties
    float radius = pickup->getRadius();
    Game* game = pickup->getGame();
    
    if (game == nullptr) {
        return nullptr; // Need game pointer to create new pickup
    }
    
    // Determine pickup type before deleting (check most specific types first)
    Heart* heart = dynamic_cast<Heart*>(pickup);
    StapleGun* stapleGun = dynamic_cast<StapleGun*>(pickup);
    Scissor* scissor = dynamic_cast<Scissor*>(pickup);
    Ladder* ladder = dynamic_cast<Ladder*>(pickup);
    
    // Remove pickup from old side's pickups list BEFORE deleting
    auto& oldPickups = fromSide->getPickups();
    for (auto it = oldPickups.begin(); it != oldPickups.end(); ++it) {
        if (*it == pickup) {
            oldPickups.erase(it);
            break;
        }
    }
    
    // Delete the old pickup (which will remove it from the old scene)
    delete pickup;
    
    // Create a new pickup instance in the new scene with preserved properties
    // NOTE: Pickups should NOT have colliders - they are created without colliders
    Pickup* newPickup = nullptr;
    
    if (heart != nullptr) {
        // Create new Heart preserving all properties
        newPickup = new Heart(game, this, {
            .mesh = mesh,
            .material = material,
            .position = position,
            .rotation = rotation,
            .scale = scale
            // NO collider - pickups don't have colliders
        }, radius);
    } else if (stapleGun != nullptr) {
        // Create new StapleGun preserving all properties
        newPickup = new StapleGun(game, this, {
            .mesh = mesh,
            .material = material,
            .position = position,
            .rotation = rotation,
            .scale = scale
            // NO collider - pickups don't have colliders
        }, radius);
    } else if (scissor != nullptr) {
        // Create new Scissor preserving all properties
        newPickup = new Scissor(game, this, {
            .mesh = mesh,
            .material = material,
            .position = position,
            .rotation = rotation,
            .scale = scale
            // NO collider - pickups don't have colliders
        }, radius);
    } else if (ladder != nullptr) {
        // Create new Ladder preserving all properties
        newPickup = new Ladder(game, this, {
            .mesh = mesh,
            .material = material,
            .position = position,
            .rotation = rotation,
            .scale = scale
            // NO collider - pickups don't have colliders
        }, radius);
    } else {
        // Fallback: create generic Pickup preserving all properties
        newPickup = new Pickup(game, this, {
            .mesh = mesh,
            .material = material,
            .position = position,
            .rotation = rotation,
            .scale = scale
            // NO collider - pickups don't have colliders
        }, radius);
    }
    
    // Restore additional properties
    if (newPickup != nullptr) {
        // Pickups are static objects, so we don't set velocity (they don't move)
        if (layer != 0.0f) { // Only set layer if it was non-zero (assuming 0 is default)
            newPickup->setLayer(layer);
        }
        addPickup(newPickup);
        return newPickup;  // Return the new pickup instance
    }
    
    return nullptr;  // Return null if creation failed
}

void SingleSide::reset() {
    // create player node
    playerNode->setPosition(playerSpawn);

    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
    
    for (Pickup* pickup : pickups) {
        delete pickup;
    }
    pickups.clear();

    for (auto spawn : enemySpawns) std::cout << spawn.x << " " << spawn.y;
    std::cout << std::endl;

    // add enemies based on biome - uniformly select from biome enemies
    if (!enemySpawns.empty()) {
        // Get enemy list for this biome
        auto biomeIt = Enemy::enemyBiomes.find(biome);
        if (biomeIt != Enemy::enemyBiomes.end()) {
            const auto& biomeEnemies = biomeIt->second;
            
            // Only proceed if we have valid enemies for this biome
            if (!biomeEnemies.empty()) {
                // Fill spawn points with enemies - each spawn has 50% chance
                for (const vec2& spawnPos : enemySpawns) {
                    // 50% chance to spawn an enemy at this location
                    if (uniform() < 0.8f) {
                        // Uniformly select from all enemies in biome
                        int randomIndex = randint(0, static_cast<int>(biomeEnemies.size() - 1));
                        const std::string& selectedEnemy = biomeEnemies[randomIndex].first;
                        
                        // Create and add the enemy
                        auto templateIt = Enemy::templates.find(selectedEnemy);
                        if (templateIt != Enemy::templates.end()) {
                            Enemy* enemy = templateIt->second(spawnPos, this);
                            if (enemy != nullptr) {
                                addEnemy(enemy);
                            }
                        }
                    }
                }
            }
        }
    }
}