#ifndef MENU_H
#define MENU_H

#include "util/includes.h"
#include "ui/uiElement.h"

class Game;

class Menu {
protected:
    Game* game;
    std::vector<Node2D*> nodes; // All visual elements
    bool visible;

public:
    Menu(Game* game);
    virtual ~Menu();

    // Core functionality
    virtual void update(float dt);
    virtual void handleEvent(const vec2& mousePos, bool mouseDown);

    // Getters
    bool isVisible() const { return visible; }
    
    // Track nodes for cleanup
    void addNode(Node2D* node) { nodes.push_back(node); }
};

#endif // MENU_H