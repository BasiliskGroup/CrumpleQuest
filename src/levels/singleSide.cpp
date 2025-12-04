#include "levels/levels.h"
#include "weapon/meleeZone.h"


SingleSide::SingleSide(Game* game, std::string mesh, std::string material) 
    : scene(nullptr), camera(nullptr), background(nullptr), playerNode(nullptr), weaponNode(nullptr)
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

void SingleSide::update(const vec2& playerPos, float dt) {
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

        // check collision
        for (int j = 0; j < enemies.size(); j++) {
            Enemy* enemy = enemies[j];
            if (enemy->isDead()) continue; // Skip already dead enemies
            if (glm::length2(enemy->getPosition() - zone->getPosition()) > (enemy->getRadius() + zone->getRadius())) continue;
            zone->hit(enemy);
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