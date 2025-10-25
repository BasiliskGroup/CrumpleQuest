#ifndef GAME_H
#define GAME_H

#include "crumpleQuest/character/enemy.h"
#include "crumpleQuest/character/player.h"
#include "crumpleQuest/levels/singleSide.h"

class Game {
private:
    Player* player;

public:
    Game();
    ~Game();

    bool update(float dt);
    const vec2& playerPos() {  } // TODO
};

#endif