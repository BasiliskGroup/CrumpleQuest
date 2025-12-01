#include "ui/menu.h"
#include "game/game.h"
#include "ui/slider.h"

Menu::Menu(Game* game) : game(game), visible(true) {
}

Menu::~Menu() {
    // Clean up all UI elements
    for (auto* element : uiElements) {
        delete element;
    }
    // Clean up all non-UI nodes (images)
    for (auto* node : nodes) {
        delete node;
    }
}

void Menu::addElement(UIElement* element) {
    uiElements.push_back(element);
    // Note: Don't add UIElements to nodes list, even if they're also Node2D
    // to avoid double-deletion. UIElements handle their own cleanup.
}

void Menu::update(float dt) {
    // Override in derived classes if needed
}

void Menu::handleEvent(const vec2& mousePos, bool mouseDown) {
    if (!visible) return;
    
    // Pass events to all UI elements
    for (UIElement* element : uiElements) {
        element->event(mousePos, mouseDown);
    }
}