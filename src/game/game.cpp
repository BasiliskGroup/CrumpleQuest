#include "levels/levels.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr),
    voidScene(nullptr),
    camera(nullptr)
{
    // basilisk preamble
    this->engine = new Engine(800, 800, "Crumple Quest");
    this->scene = new Scene2D(this->engine);
    this->scene->getSolver()->setGravity(0);
    this->camera = new StaticCamera2D(engine);
    this->scene->setCamera(this->camera);
    
    // storing templates
    this->voidScene = new Scene2D(this->engine);

    // initialize first paper
    this->paper = new Paper();
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

    delete paper; paper = nullptr;

    // basilisk closing, must be last
    delete engine; engine = nullptr;
    delete scene; scene = nullptr;
    delete camera; camera = nullptr;
    delete voidScene; voidScene = nullptr;
}

void Game::update(float dt) {
    // folding
    bool leftIsDown = engine->getMouse()->getLeftDown();
    vec2 mousePos = { engine->getMouse()->getX(), engine->getMouse()->getY() };

    if (!leftWasDown && leftIsDown) { // we just clicked
        LeftStartDown = mousePos;
        std::cout << mousePos.x << ", " << mousePos.y << std::endl;

    } else if (leftWasDown && !leftIsDown) { // we just let go
        std::cout << mousePos.x << ", " << mousePos.y << std::endl;
        
    }
    leftWasDown = leftIsDown;

    // entity update
    if (player != nullptr) {
        player->move(dt);
    }

    for (Enemy* enemy : enemies) {
        enemy->move(player->getPosition(), dt);
    }

    // add paper stuff to the scene

    // basilisk update
    engine->update();
    scene->update(dt);
    scene->render();
    engine->render();

    // delete paper stuff 
}
