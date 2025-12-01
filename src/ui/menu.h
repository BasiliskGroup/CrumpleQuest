#ifndef MENU_H
#define MENU_H

#include "util/includes.h"
#include "ui/uiElement.h"

class Game;

class Menu {
protected:
    Game* game;
    std::vector<UIElement*> interactiveElements; // Buttons, sliders, etc that receive events
    std::vector<Node2D*> nodes; // All visual elements (buttons, titles, backgrounds, etc.)
    std::vector<Mesh*> originalMeshes; // Store original meshes for show/hide
    bool visible;

public:
    Menu(Game* game);
    virtual ~Menu();

    // Core functionality
    virtual void show();
    virtual void hide();
    virtual void update(float dt);
    virtual void handleEvent(const vec2& mousePos, bool mouseDown);

    // Getters
    bool isVisible() const { return visible; }

    // Add elements to the menu
    void addNode(Node2D* node); // Add any visual element
    void addInteractive(UIElement* element); // Add interactive element (also adds as node if it's Node2D)
};

#endif // MENU_H