#ifndef PLAYER_H
#define PLAYER_H

#include "character/character.h"
#include "resource/animator.h"
#include "resource/animation.h"
#include <unordered_map>


class Player : public Character {
private:
    int money = 0;
    Animator* animator;
    std::unordered_map<std::string, Animation*>* animations;

public:
    Player(int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, std::unordered_map<std::string, Animation*>* animations);
    ~Player() = default;
    
    void onDamage(int damage);
    void move(float dt);
};

#endif