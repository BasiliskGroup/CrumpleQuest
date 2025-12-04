#ifndef ENEMY_H
#define ENEMY_H

#include "character/character.h"
#include "character/ai.h"
#include "character/behavior.h"
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
    float attacking = 0.0f;  // Timer for attack animation
    Behavior* behavior = nullptr;  // Current behavior
    std::function<Behavior*(const vec2&, float)> behaviorSelector;  // Function that selects which behavior to use
    
    // Status flags (updated by updateStatus)
    bool statusHasLineOfSight = false;
    bool statusCanAttack = false;
    bool statusIsAttacking = false;
    bool statusHasPath = false;
    int statusNumEnemiesOnSide = 0;
    bool statusWeaponReady = false;

public:
    Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai, float radius, vec2 scale, std::string hitSound = "hit");
    ~Enemy() = default;

    void onDamage(int damage);
    void updateStatus(const vec2& playerPos);
    virtual void move(const vec2& playerPos, float dt);
    void attack(const vec2& playerPos, float dt);

    void setPath(std::vector<vec2> path) { this->path = path; }
    std::vector<vec2>& getPath() { return path; }
    AI* getAI() { return ai; }
    float getFinishRadius() const { return finishRadius; }
    float& getAttacking() { return attacking; }
    Animator* getAnimator() { return animator; }
    Behavior* getBehavior() { return behavior; }
    void setBehavior(Behavior* behavior) { this->behavior = behavior; }
    void setBehavior(const std::string& behaviorName);  // Set behavior by name from registry
    
    // Behavior selector getter/setter
    std::function<Behavior*(const vec2&, float)>& getBehaviorSelector() { return behaviorSelector; }
    void setBehaviorSelector(const std::function<Behavior*(const vec2&, float)>& func) { behaviorSelector = func; }
    
    // Status getters
    bool hasLineOfSightStatus() const { return statusHasLineOfSight; }
    bool canAttackStatus() const { return statusCanAttack; }
    bool isAttackingStatus() const { return statusIsAttacking; }
    bool hasPathStatus() const { return statusHasPath; }
    int numEnemiesOnSideStatus() const { return statusNumEnemiesOnSide; }
    bool weaponReadyStatus() const { return statusWeaponReady; }
    
    void updateNode(Node2D* newNode); // Update both the Character's node and the animator's node
};

#endif