#include "levels/levels.h"
#include "weapon/meleeZone.h"


SingleSide::SingleSide(Game* game, std::string mesh, std::string material) 
    : scene(nullptr), camera(nullptr), background(nullptr), playerNode(nullptr)
{
    scene = new Scene2D(game->getEngine());
    this->camera = new StaticCamera2D(game->getEngine());
    this->scene->setCamera(this->camera);
    this->camera->setScale(12.0f, 9.0f);
    this->scene->getSolver()->setGravity(0);

    loadResources();

    // create player node
    playerNode = SingleSide::genPlayerNode(game, this);

    // create background node
    background = new Node2D(scene, { 
        .mesh=game->getMesh(mesh), 
        .material=game->getMaterial(material), 
        .scale={ 1, 1 } 
    });
    background->setLayer(-0.7);
}

SingleSide::SingleSide(const SingleSide& other) noexcept : scene(nullptr), camera(nullptr), background(nullptr) {
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
}

SingleSide::SingleSide(SingleSide&& other) noexcept : 
    scene(other.scene),
    camera(other.camera),
    enemies(std::move(other.enemies)),
    damageZones(std::move(other.damageZones)), 
    background(std::move(other.background))
{
    other.scene = nullptr;
    other.camera = nullptr;
    other.background = nullptr;

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