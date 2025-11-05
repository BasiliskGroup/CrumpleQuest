#include "game/game.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr),
    camera(nullptr)
{
    this->engine = new Engine(800, 800, "Crumple Quest");
    this->scene = new Scene2D(this->engine);
    this->camera = new StaticCamera2D(engine);
    
    this->scene->getSolver()->setGravity(0);
    this->scene->setCamera(this->camera);
}

Game::~Game() {
    delete player; player = nullptr;
    delete floor; floor = nullptr;

    // materials
    for (auto const& [name, material] : materials) {
        delete material;
    }
    materials.clear();

    // images
    for (auto const& [name, image] : images) {
        delete image;
    }
    images.clear();

    // meshes
    for (auto const& [name, mesh] : meshes) {
        delete mesh;
    }
    meshes.clear();

    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    delete engine; engine = nullptr;
    delete scene; scene = nullptr;
    delete camera; camera = nullptr;
}

void Game::update(float dt) {
    // entity update
    if (player != nullptr) {
        player->move(dt);
    }

    for (Enemy* enemy : enemies) {
        enemy->move(player->getPosition(), dt);
    }

    // basilisk update
    engine->update();
    scene->update(dt);
    scene->render();
    engine->render();
}
