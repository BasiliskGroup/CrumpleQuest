#include "levels/levels.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr),
    voidScene(nullptr),
    camera(nullptr)
{
    std::cout << "Game constructor started" << std::endl;
    std::cout << "Creating Engine..." << std::endl;
    this->engine = new Engine(800, 800, "Crumple Quest");
    std::cout << "Engine created: " << this->engine << std::endl;
    
    std::cout << "Creating Scene2D..." << std::endl;
    this->scene = new Scene2D(this->engine);
    std::cout << "Scene created: " << this->scene << std::endl;
    
    std::cout << "Creating Camera..." << std::endl;
    this->camera = new StaticCamera2D(engine);
    std::cout << "Camera created: " << this->camera << std::endl;
    
    std::cout << "Creating VoidScene..." << std::endl;
    this->voidScene = new Scene2D(this->engine);
    std::cout << "VoidScene created: " << this->voidScene << std::endl;
    
    std::cout << "Setting gravity..." << std::endl;
    this->scene->getSolver()->setGravity(0);
    std::cout << "Setting camera..." << std::endl;
    this->scene->setCamera(this->camera);
    
    std::cout << "Game constructor finished" << std::endl;
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
    delete voidScene; voidScene = nullptr;
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
