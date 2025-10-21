#ifndef GAME_H
#define GAME_H

#include "crumpleQuest/character/enemy.h"
#include "crumpleQuest/character/player.h"
#include "crumpleQuest/levels/singleSide.h"
#include "engine/engine.h"

class Game {
private:
    Engine* engine;
    Player* player;

public:
    Game();
    ~Game();

    bool update(float dt);
};

#endif