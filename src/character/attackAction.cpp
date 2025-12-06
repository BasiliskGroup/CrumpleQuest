#include "character/attackAction.h"
#include "character/attackActions.h"

std::unordered_map<std::string, AttackAction*> AttackActionRegistry::actions;

void AttackActionRegistry::registerAction(const std::string& name, AttackAction* action) {
    actions[name] = action;
}

AttackAction* AttackActionRegistry::getAction(const std::string& name) {
    auto it = actions.find(name);
    if (it != actions.end()) {
        return it->second;
    }
    return nullptr;
}

void AttackActionRegistry::initialize() {
    // Register all available attack actions
    registerAction("Normal", new NormalAttackAction());
    registerAction("Burst", new BurstAttackAction());
}

void AttackActionRegistry::cleanup() {
    for (auto& pair : actions) {
        delete pair.second;
    }
    actions.clear();
}
