#ifndef ENEMY_H
#define ENEMY_H

#include "character/character.h"
#include "character/ai.h"

class Enemy : public Character {
private:
    AI* ai;
    

public:
    Enemy(int health, float speed, Node2D* node, Weapon* weapon, AI* ai);
    ~Enemy();

    void onDamage(int damage);
    void move(float dt);
};

#endif