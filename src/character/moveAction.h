#ifndef MOVE_ACTION_H
#define MOVE_ACTION_H

#include "util/includes.h"

class Enemy;

// Base class for enemy move actions
class MoveAction {
public:
    virtual ~MoveAction() = default;
    
    // Execute the move action toward a destination
    // destination: target position to move toward
    virtual void execute(Enemy* enemy, const vec2& destination) = 0;
    
    // Update the action (for maintaining timers, etc.)
    // Called each frame
    virtual void update(Enemy* enemy, float dt) {}
    
    // Get a display name for this action (for debugging)
    virtual std::string getName() const = 0;
};

// Static registry for move actions
class MoveActionRegistry {
public:
    static std::unordered_map<std::string, MoveAction*> actions;
    
    // Register an action with a name
    static void registerAction(const std::string& name, MoveAction* action);
    
    // Get an action by name (returns nullptr if not found)
    static MoveAction* getAction(const std::string& name);
    
    // Initialize default actions
    static void initialize();
    
    // Cleanup all registered actions
    static void cleanup();
};

#endif
