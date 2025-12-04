#include "character/behavior.h"
#include "character/enemy.h"
#include "character/behaviors.h"

std::unordered_map<std::string, Behavior*> BehaviorRegistry::behaviors;

void BehaviorRegistry::registerBehavior(const std::string& name, Behavior* behavior) {
    behaviors[name] = behavior;
}

Behavior* BehaviorRegistry::getBehavior(const std::string& name) {
    auto it = behaviors.find(name);
    if (it != behaviors.end()) {
        return it->second;
    }
    return nullptr;
}

void BehaviorRegistry::initialize() {
    // Register all available behaviors
    registerBehavior("ChasePlayer", new ChasePlayerBehavior());
    registerBehavior("Runaway", new RunawayBehavior());
    registerBehavior("Idle", new IdleBehavior());
    registerBehavior("Stationary", new StationaryBehavior());
}

void BehaviorRegistry::cleanup() {
    for (auto& pair : behaviors) {
        delete pair.second;
    }
    behaviors.clear();
}

