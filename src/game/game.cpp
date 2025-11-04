#include "game/game.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr)
{
    this->engine = new Engine(800, 600, "Crumple Quest");
    this->scene = new Scene2D(this->engine);
    this->player = new Player(3, nullptr, nullptr); // TODO add weapon
    // this->floor = new Floor();
    
}

Game::~Game() {
    delete player;
    delete floor;
    delete engine;
    delete scene;

    player = nullptr;
    engine = nullptr;
    floor = nullptr;
    scene = nullptr;
}

bool Game::update(float dt) {
    // entity update
    // player->move(dt);

    // basilisk update
    engine->update();
    scene->update(dt);
    scene->render();
    engine->render();
}

const vec2& Game::playerPos() {
    
}
