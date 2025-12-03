#ifndef ENEMY_H
#define ENEMY_H

#include "character/character.h"
#include "character/ai.h"

class Enemy : public Character {
private:
    AI* ai;
    std::vector<vec2> path;
    float finishRadius = 0.2;

public:
    Enemy(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai);
    ~Enemy() = default;

    void onDamage(int damage);
    void move(const vec2& playerPos, float dt);

    void setPath(std::vector<vec2> path) { this->path = path; }
};

#endif