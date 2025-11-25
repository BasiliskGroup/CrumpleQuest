#ifndef PLAYER_H
#define PLAYER_H

#include "character/character.h"

class Player : public Character {
private:
    int money = 0;

public:
    Player(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon);
    
    void onDamage(int damage);
    void move(float dt);
};

#endif