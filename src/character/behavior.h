#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include "util/includes.h"

class Enemy;

// Base class for enemy behaviors
class Behavior {
public:
    virtual ~Behavior() = default;
    
    // Update the enemy's behavior - called each frame
    // Returns true if behavior should continue, false if it should switch
    virtual void update(Enemy* enemy, const vec2& playerPos, float dt) = 0;
    
    // Get a display name for this behavior (for debugging)
    virtual std::string getName() const = 0;
};

// Static registry for behaviors
class BehaviorRegistry {
public:
    static std::unordered_map<std::string, Behavior*> behaviors;
    
    // Register a behavior with a name
    static void registerBehavior(const std::string& name, Behavior* behavior);
    
    // Get a behavior by name (returns nullptr if not found)
    static Behavior* getBehavior(const std::string& name);
    
    // Initialize default behaviors
    static void initialize();
    
    // Cleanup all registered behaviors
    static void cleanup();
};

#endif

