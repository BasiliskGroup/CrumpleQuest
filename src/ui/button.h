#ifndef BUTTON_H
#define BUTTON_H

#include "util/includes.h"
#include "ui/uiElement.h"

class Game;

class Button : public Node2D, public UIElement {
private:
    struct Params {
        std::function<void()> onDown = []() {};
        std::function<void()> onUp = []() {};
        std::function<void()> onHover = []() {};
    };

    std::function<void()> onDown;
    std::function<void()> onUp;
    std::function<void()> onHover;

    bool wasPressed = false;

    Game* game;

public:
    Button(Game* game, Node2D::Params params, Button::Params callbacks);
    ~Button() = default;
    
    // Rule of 5
    Button(const Button& other); 
    Button& operator=(const Button& other); 
    Button(Button&& other) noexcept;      
    Button& operator=(Button&& other) noexcept;  

    bool isHovered(const vec2& pos);
    
    void event(const vec2& pos, bool mouseDown) override;
    void setOnDown(std::function<void()> onDown) { this->onDown = onDown; }
    void setOnUp(std::function<void()> onUp) { this->onUp = onUp; }
    void setOnHover(std::function<void()> onHover) { this->onHover = onHover; }
};

#endif