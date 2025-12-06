#ifndef MENU_H
#define MENU_H

#include "util/includes.h"
#include "ui/uiElement.h"

class Game;

class Menu {
protected:
    Game* game;
    std::vector<Node2D*> nodes; // All visual elements
    std::vector<UIElement*> uiElements; // Interactive elements (buttons, sliders, etc.)
    bool visible;
    
    // Animation state
    std::unordered_map<Node2D*, float> targetYPositions; // Target Y for each node (elements and plain nodes)
    float animationTimer; // Time elapsed since menu appeared/disappeared
    float animationDuration; // How long the slide-in/out takes
    float slideDistance; // How far below screen to start/end
    bool animatingOut; // True if animating out (closing), false if animating in (opening)
    bool isClosing; // True if menu has been marked for closing

public:
    Menu(Game* game);
    virtual ~Menu();

    // Core functionality
    virtual void update(float dt);
    virtual void handleEvent(const vec2& mousePos, bool mouseDown);

    // Getters
    bool isVisible() const { return visible; }
    bool isDoneAnimating() const { return animationTimer >= animationDuration; }
    bool isAnimatingOut() const { return animatingOut; }
    
    // Add UI elements (buttons, sliders, etc) or plain nodes
    void addElement(UIElement* element);
    void addNode(Node2D* node);
    
    // Animation control
    void resetAnimation();
    void startCloseAnimation();
};

#endif // MENU_H