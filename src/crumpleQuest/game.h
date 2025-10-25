#ifndef GAME_H
#define GAME_H

#include "crumpleQuest/character/enemy.h"
#include "crumpleQuest/character/player.h"
#include "crumpleQuest/levels/singleSide.h"

class Game {
private:
    Player* player;
    // std::vector<Paper>

public:
    Game();
    ~Game();

    bool update(float dt);
};

#endif