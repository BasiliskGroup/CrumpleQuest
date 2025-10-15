#ifndef PLAYER_H
#define PLAYER_H

#include "crumpleQuest/character/character.h"

class Player : public Character {
private:


public:
    Player(int health, Weapon* weapon);
    
};

#endif