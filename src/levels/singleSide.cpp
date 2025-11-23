#include "levels/levels.h"

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
}

SingleSide::SingleSide(SingleSide&& other) noexcept : 
    scene(other.scene),
    camera(other.camera),
    enemies(std::move(other.enemies))
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
    return *this;
}

SingleSide& SingleSide::operator=(SingleSide&& other) noexcept {
    if (this == &other) return *this;
    clear();

    // transfer
    scene = other.scene;
    camera = other.camera;
    enemies = std::move(other.enemies);

    // clear other
    other.scene = nullptr;
    other.camera = nullptr;
    return *this;
}

void SingleSide::generateNavmesh() {
    
}

void SingleSide::update(float dt) {
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