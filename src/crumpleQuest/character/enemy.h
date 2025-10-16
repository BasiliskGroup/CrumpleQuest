#ifndef ENEMY_H
#define ENEMY_H

#include "crumpleQuest/character/character.h"
#include "crumpleQuest/character/ai.h"

class Enemy : public Character {
private:
    AI* ai;

public:
    Enemy(int health, Weapon* weapon, AI* ai);
    ~Enemy();

    void onDamage(int damage);
    void move() override;
};

#endif