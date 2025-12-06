#include "levels/levels.h"
#include "weapon/meleeZone.h"
#include "util/random.h"


SingleSide::SingleSide(Game* game, std::string mesh, std::string material, vec2 playerSpawn, std::string biome, std::vector<vec2> enemySpawns, float difficulty) : 
    scene(nullptr), 
    camera(nullptr), 
    background(nullptr), 
    playerNode(nullptr), 
    weaponNode(nullptr), 
    playerSpawn(playerSpawn),
    biome(biome)
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

    // add enemies based on difficulty
    if (!enemySpawns.empty() && difficulty > 0.0f) {
        // Get enemy list for this biome
        auto biomeIt = Enemy::enemyBiomes.find(biome);
        if (biomeIt != Enemy::enemyBiomes.end()) {
            const auto& biomeEnemies = biomeIt->second;
            
            // Find the weakest enemy (lowest price) for fallback
            std::string weakestEnemy;
            float weakestPrice = std::numeric_limits<float>::max();
            if (!biomeEnemies.empty()) {
                for (const auto& enemyPair : biomeEnemies) {
                    if (enemyPair.second < weakestPrice) {
                        weakestPrice = enemyPair.second;
                        weakestEnemy = enemyPair.first;
                    }
                }
            }
            
            // Only proceed if we have valid enemies for this biome
            if (!weakestEnemy.empty()) {
                float remainingDifficulty = difficulty;
                
                // Shuffle spawn points for randomness
                std::vector<vec2> shuffledSpawns = enemySpawns;
                for (size_t i = shuffledSpawns.size() - 1; i > 0; --i) {
                    size_t j = randint(0, static_cast<int>(i));
                    std::swap(shuffledSpawns[i], shuffledSpawns[j]);
                }
                
                // Fill spawn points with enemies
                for (const vec2& spawnPos : shuffledSpawns) {
                    std::string selectedEnemy;
                    float selectedPrice = 0.0f;
                    
                    // Filter enemies that fit in remaining difficulty
                    std::vector<std::pair<std::string, float>> affordableEnemies;
                    for (const auto& enemyPair : biomeEnemies) {
                        if (enemyPair.second <= remainingDifficulty) {
                            affordableEnemies.push_back(enemyPair);
                        }
                    }
                    
                    if (!affordableEnemies.empty()) {
                        // Randomly select from affordable enemies (weighted by price)
                        // Higher price enemies are more likely to be selected
                        float totalWeight = 0.0f;
                        for (const auto& enemyPair : affordableEnemies) {
                            totalWeight += enemyPair.second;
                        }
                        
                        float randomValue = uniform(0.0f, totalWeight);
                        float cumulativeWeight = 0.0f;
                        for (const auto& enemyPair : affordableEnemies) {
                            cumulativeWeight += enemyPair.second;
                            if (randomValue <= cumulativeWeight) {
                                selectedEnemy = enemyPair.first;
                                selectedPrice = enemyPair.second;
                                break;
                            }
                        }
                    } else {
                        // No affordable enemies, use weakest enemy
                        selectedEnemy = weakestEnemy;
                        selectedPrice = weakestPrice;
                    }
                    
                    // Create and add the enemy
                    auto templateIt = Enemy::templates.find(selectedEnemy);
                    if (templateIt != Enemy::templates.end()) {
                        Enemy* enemy = templateIt->second(spawnPos, this);
                        if (enemy != nullptr) {
                            addEnemy(enemy);
                            remainingDifficulty -= selectedPrice;
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
    weaponNode(nullptr)
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

    scene->update();
}

void SingleSide::clear() {
    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();
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