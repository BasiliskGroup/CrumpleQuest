#ifndef PLAYER_H
#define PLAYER_H

#include "crumpleQuest/character/character.h"

class Player : public Character {
private:
    int money = 0;

public:
    Player(int health, Weapon* weapon);
    
    void onDamage(int damage);
    void move() override;
};

#endif