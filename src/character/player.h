#ifndef PLAYER_H
#define PLAYER_H

#include "character/character.h"

class Player : public Character {
private:
    int money = 0;

public:
    Player(int health, Node2D* node, Weapon* weapon);
    
    void onDamage(int damage);
    void move(float dt);
};

#endif