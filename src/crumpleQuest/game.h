#ifndef GAME_H
#define GAME_H

#include "crumpleQuest/character/enemy.h"
#include "crumpleQuest/character/player.h"

class Game {
private:
    Player* player;

public:
    Game();
    ~Game();

    bool update(float dt);
};

#endif