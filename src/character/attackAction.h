#ifndef ATTACK_ACTION_H
#define ATTACK_ACTION_H

#include "util/includes.h"

class Enemy;

// Base class for enemy attack actions
class AttackAction {
public:
    virtual ~AttackAction() = default;
    
    // Execute the attack action
    // pos: position to attack from
    // dir: direction to attack in
    // Returns true if attack was successful
    virtual bool execute(Enemy* enemy, const vec2& pos, const vec2& dir) = 0;
    
    // Update the action (for maintaining timers, etc.)
    // Called each frame
    virtual void update(Enemy* enemy, float dt) {}
    
    // Get a display name for this action (for debugging)
    virtual std::string getName() const = 0;
};

// Static registry for attack actions
class AttackActionRegistry {
public:
    static std::unordered_map<std::string, AttackAction*> actions;
    
    // Register an action with a name
    static void registerAction(const std::string& name, AttackAction* action);
    
    // Get an action by name (returns nullptr if not found)
    static AttackAction* getAction(const std::string& name);
    
    // Initialize default actions
    static void initialize();
    
    // Cleanup all registered actions
    static void cleanup();
};

#endif
