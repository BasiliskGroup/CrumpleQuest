#include "game/game.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr)
{
    this->engine = new Engine(800, 800, "Crumple Quest");
    this->scene = new Scene2D(this->engine);    
    this->scene->getSolver()->setGravity(0);
}

Game::~Game() {
    delete player;
    delete floor;
    delete engine;
    delete scene;

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

    player = nullptr;
    engine = nullptr;
    floor = nullptr;
    scene = nullptr;
}

bool Game::update(float dt) {
    // entity update
    if (player != nullptr) {
        player->move(dt);
    }

    // basilisk update
    engine->update();
    scene->update(dt);
    scene->render();
    engine->render();
}

const vec2& Game::playerPos() {
    
}
