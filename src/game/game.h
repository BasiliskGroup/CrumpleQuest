#ifndef GAME_H
#define GAME_H

#include "util/includes.h"
#include "character/player.h"
#include "levels/levels.h"

class Game {
private:
    Engine* engine;
    Scene2D* scene;

    Player* player;
    Floor* floor;

public:
    Game();
    ~Game();

    Engine*& getEngine() { return engine; }
    Scene2D*& getScene() { return scene; }

    bool update(float dt);
    const vec2& playerPos();
};

#endif