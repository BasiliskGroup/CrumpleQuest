#include "character/moveAction.h"
#include "character/moveActions.h"

std::unordered_map<std::string, MoveAction*> MoveActionRegistry::actions;

void MoveActionRegistry::registerAction(const std::string& name, MoveAction* action) {
    actions[name] = action;
}

MoveAction* MoveActionRegistry::getAction(const std::string& name) {
    auto it = actions.find(name);
    if (it != actions.end()) {
        return it->second;
    }
    return nullptr;
}

void MoveActionRegistry::initialize() {
    // Register all available move actions
    registerAction("Normal", new NormalMoveAction());
    registerAction("Jump", new JumpMoveAction(2.0f, 10.0f));  // 2 second wait, 10x speed dash
}

void MoveActionRegistry::cleanup() {
    for (auto& pair : actions) {
        delete pair.second;
    }
    actions.clear();
}
