#include "ui/menu.h"
#include "game/game.h"

Menu::Menu(Game* game) : game(game), visible(false) {
}

Menu::~Menu() {
    // Clean up all nodes (this includes interactive elements that inherit from Node2D)
    for (auto* node : nodes) {
        delete node;
    }
}

void Menu::show() {
    visible = true;
    
    // Restore all node meshes
    for (size_t i = 0; i < nodes.size(); i++) {
        nodes[i]->setMesh(originalMeshes[i]);
    }
}

void Menu::hide() {
    visible = false;
    
    // Set all node meshes to nullptr
    for (auto* node : nodes) {
        node->setMesh(nullptr);
    }
}

void Menu::update(float dt) {
    // Override in derived classes if needed
}

void Menu::handleEvent(const vec2& mousePos, bool mouseDown) {
    if (!visible) return;
    
    // Pass events to interactive elements only
    for (auto* element : interactiveElements) {
        element->event(mousePos, mouseDown);
    }
}

void Menu::addNode(Node2D* node) {
    nodes.push_back(node);
    originalMeshes.push_back(node->getMesh());
}

void Menu::addInteractive(UIElement* element) {
    interactiveElements.push_back(element);
    // If it's also a Node2D, add it to the visual nodes
    if (Node2D* node = dynamic_cast<Node2D*>(element)) {
        addNode(node);
    }
}