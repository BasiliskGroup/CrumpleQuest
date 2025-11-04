#include "game/game.h"

Game::Game(): player(nullptr), floor(nullptr) {
    this->player = new Player(3, nullptr); // TODO add weapon
    this->floor = new Floor();
}

Game::~Game() {

}

bool Game::update(float dt) {

}

const vec2& Game::playerPos() {
    
}
