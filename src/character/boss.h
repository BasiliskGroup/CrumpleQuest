#ifndef BOSS_H
#define BOSS_H

#include "character/character.h"

class Game;
class SingleSide;
class Weapon;

class Boss : public Character {
private:
    vec2 hitboxOffset;
    std::string stage;

public:
    Boss(Game* game, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 scale, vec2 hitboxOffset);
    ~Boss();
};

#endif