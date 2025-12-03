#ifndef PLAYER_H
#define PLAYER_H

#include "character/character.h"
#include "resource/animator.h"
#include "resource/animation.h"
#include "ui/menu_manager.h"
#include <unordered_map>


class Player : public Character {
private:
    int money = 0;
    float attacking = 0;
    Animator* animator;
    Node2D* weaponNode;
    Animator* weaponAnimator;

public:
    Player(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon);
    ~Player() = default;
    
    void onDamage(int damage);
    void move(float dt);
};

#endif