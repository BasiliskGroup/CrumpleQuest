#ifndef GAME_H
#define GAME_H

#include "character/enemy.h"
#include "character/player.h"
#include "levels/singleSide.h"

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