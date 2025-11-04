#ifndef GAME_H
#define GAME_H

#include "character/player.h"
#include "levels/levels.h"

class Game {
private:
    Player* player;
    Floor* floor;

public:
    Game();
    ~Game();

    bool update(float dt);
    const vec2& playerPos();
};

#endif