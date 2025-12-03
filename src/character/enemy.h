#ifndef ENEMY_H
#define ENEMY_H

#include "character/character.h"
#include "character/ai.h"
#include "resource/animation.h"
#include "resource/animator.h"

class Game;
class SingleSide;

class Enemy : public Character {
public:
    static std::unordered_map<std::string, std::function<Enemy*(vec2, SingleSide*)>> templates;
    static void generateTemplates(Game* game);
    Animation* idleAnimation;
    Animation* runAnimation;
    Animation* attackAnimation;

private:
    AI* ai;
    Animator* animator;
    std::vector<vec2> path;
    float finishRadius = 0.2;

public:
    Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai);
    ~Enemy() = default;

    void onDamage(int damage);
    void move(const vec2& playerPos, float dt);
    void attack(const vec2& playerPos, float dt);

    void setPath(std::vector<vec2> path) { this->path = path; }
};

#endif