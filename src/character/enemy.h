#ifndef ENEMY_H
#define ENEMY_H

#include "character/character.h"
#include "character/ai.h"
#include "character/behavior.h"
#include "resource/animation.h"
#include "resource/animator.h"
#include <optional>

class Game;
class SingleSide;
class Weapon;

class Enemy : public Character {
public:
    static std::unordered_map<std::string, std::function<Enemy*(vec2, SingleSide*)>> templates;
    static void generateTemplates(Game* game);
    Animation* idleAnimation;
    Animation* runAnimation;
    Animation* attackAnimation;
    
    // Multi-shot attack system
    struct PendingShot {
        vec2 pos;
        vec2 dir;
        float timer;
        Weapon* weapon;  // Weapon to use for this shot (bypasses cooldown)
    };

private:
    AI* ai;
    Animator* animator;
    std::vector<vec2> path;
    float finishRadius = 0.2;
    float attacking = 0.0f;  // Timer for attack animation
    float attackDelay = 0.0f;  // Delay before weapon attack is called after animation starts
    float attackDelayTimer = 0.0f;  // Timer tracking the delay
    bool attackPending = false;  // Whether an attack is waiting for delay
    vec2 pendingAttackPos;  // Position to attack when delay completes
    vec2 pendingAttackDir;  // Direction to attack when delay completes
    
    std::vector<PendingShot> pendingShots;  // Queue of shots to fire with delays
    
    float wanderDestinationTimer = 0.0f;  // Timer for wander destination refresh
    
    Behavior* behavior = nullptr;  // Current behavior
    std::function<Behavior*(const vec2&, float)> behaviorSelector;  // Function that selects which behavior to use
    std::function<bool(vec2, vec2)> attackAction;  // Function that performs the attack action
    std::function<void(const vec2&)> moveAction;  // Function that applies movement toward a destination
    std::optional<vec2> customDestination;  // Optional custom destination (if set, updatePathing will skip this enemy)
    
    // Status flags (updated by updateStatus)
    bool statusHasLineOfSight = false;
    bool statusCanAttack = false;
    bool statusIsAttacking = false;
    bool statusHasPath = false;
    int statusNumEnemiesOnSide = 0;
    bool statusWeaponReady = false;

public:
    Enemy(Game* game, int health, float speed, Node2D* node, SingleSide* side, Weapon* weapon, AI* ai, float radius, vec2 scale, std::string hitSound = "hit", float attackDelay = 0.0f);
    ~Enemy() = default;

    void onDamage(int damage) override;
    void onDeath() override;
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
    
    // Attack action getter/setter
    std::function<bool(vec2, vec2)>& getAttackAction() { return attackAction; }
    void setAttackAction(const std::function<bool(vec2, vec2)>& func) { attackAction = func; }
    
    // Move action getter/setter
    std::function<void(const vec2&)>& getMoveAction() { return moveAction; }
    void setMoveAction(const std::function<void(const vec2&)>& func) { moveAction = func; }
    
    // Pending shots getter (for multi-shot attacks)
    std::vector<PendingShot>& getPendingShots() { return pendingShots; }
    
    // Wander destination timer getter
    float& getWanderDestinationTimer() { return wanderDestinationTimer; }
    
    // Custom destination getter/setter (used to override default pathfinding to player)
    std::optional<vec2> getCustomDestination() const { return customDestination; }
    void setCustomDestination(const vec2& dest) { customDestination = dest; }
    void clearCustomDestination() { customDestination.reset(); }
    
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