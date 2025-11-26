#include "levels/levels.h"
#include "weapon/meleeZone.h"


SingleSide::SingleSide(Game* game) : scene(nullptr), camera(nullptr) {
    scene = new Scene2D(game->getEngine());
    this->camera = new StaticCamera2D(game->getEngine());
    this->camera->setScale(9.0f);
    this->scene->setCamera(this->camera);
    this->scene->getSolver()->setGravity(0);
}

SingleSide::SingleSide(const SingleSide& other) noexcept : scene(nullptr), camera(nullptr) {
    if (other.scene) scene = new Scene2D(*other.scene);
    if (other.camera) camera = new StaticCamera2D(*other.camera);
    enemies = other.enemies;
    damageZones = other.damageZones;
}

SingleSide::SingleSide(SingleSide&& other) noexcept : 
    scene(other.scene),
    camera(other.camera),
    enemies(std::move(other.enemies)),
    damageZones(std::move(other.damageZones))
{
    other.scene = nullptr;
    other.camera = nullptr;
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
        for (Enemy* enemy : enemies) {
            if (glm::length2(enemy->getPosition() - zone->getPosition()) > (enemy->getRadius() + zone->getRadius())) continue;
            zone->hit(enemy);
        }
    }

    // remove all dead enemies
    for (int i = 0; i < enemies.size(); i++) {
        Enemy* enemy = enemies[i];
        if (enemy->isDead() == false) continue;

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
    scene->render();
}

void SingleSide::clear() {
    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    delete scene; scene = nullptr;
    delete camera; camera = nullptr;
}