#include "ui/menu.h"
#include "game/game.h"

Menu::Menu(Game* game) : game(game), visible(true) {
}

Menu::~Menu() {
    // Clean up all nodes
    for (auto* node : nodes) {
        delete node;
    }
}

void Menu::update(float dt) {
    // Override in derived classes if needed
}

void Menu::handleEvent(const vec2& mousePos, bool mouseDown) {
    if (!visible) return;
    
    // Pass events to all nodes
    for (Node2D* node : nodes) {
        if (UIElement* element = dynamic_cast<UIElement*>(node)) {
            element->event(mousePos, mouseDown);
        }
    }
}