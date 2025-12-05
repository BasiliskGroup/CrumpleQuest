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
    unsigned int lastAttackFrame = 0;  // Track last frame of attack animation
    Animator* animator;
    Node2D* weaponNode;
    Animator* weaponAnimator;

public:
    Player(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 sacle);
    ~Player() = default;
    
    void onDamage(int damage) override;
    void onDeath() override;
    void move(float dt);

    Node2D* getNode() { return node; }
    Node2D* getWeaponNode() { return weaponNode; }

    void setNodes(Node2D* node, Node2D* weapon);
};

#endif