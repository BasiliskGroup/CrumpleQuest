#include "ui/ui.h"

Button::Button(Scene2D* scene, Game* game, Node2D::Params params, Button::Params callbacks) : 
    Node2D(scene, params),
    onDown(callbacks.onDown),
    onUp(callbacks.onUp),
    onHover(callbacks.onHover),
    game(game)
{
}

// Copy constructor
Button::Button(const Button& other) :
    Node2D(other),  // Copy base class
    onDown(other.onDown),
    onUp(other.onUp),
    onHover(other.onHover),
    wasPressed(false),  // Don't copy pressed state
    game(other.game)
{
}

// Copy assignment operator
Button& Button::operator=(const Button& other) {
    if (this != &other) {
        // Copy base class
        Node2D::operator=(other);
        
        // Copy members
        onDown = other.onDown;
        onUp = other.onUp;
        onHover = other.onHover;
        wasPressed = false;  // Don't copy pressed state
        game = other.game;
    }
    return *this;
}

// Move constructor
Button::Button(Button&& other) noexcept :
    Node2D(std::move(other)),  // Move base class
    onDown(std::move(other.onDown)),
    onUp(std::move(other.onUp)),
    onHover(std::move(other.onHover)),
    wasPressed(other.wasPressed),
    game(other.game)
{
    // Reset other's state
    other.wasPressed = false;
    other.game = nullptr;
}

// Move assignment operator
Button& Button::operator=(Button&& other) noexcept {
    if (this != &other) {
        // Move base class
        Node2D::operator=(std::move(other));
        
        // Move members
        onDown = std::move(other.onDown);
        onUp = std::move(other.onUp);
        onHover = std::move(other.onHover);
        wasPressed = other.wasPressed;
        game = other.game;
        
        // Reset other's state
        other.wasPressed = false;
        other.game = nullptr;
    }
    return *this;
}

bool Button::isHovered(const vec2& pos) {
    return (this->position.x - this->scale.x < pos.x && pos.x < this->position.x + this->scale.x) 
        && (this->position.y - this->scale.y < pos.y && pos.y < this->position.y + this->scale.y);
}

void Button::event(const vec2& pos, bool mouseDown) {
    if (isHovered(pos) == false) {
        wasPressed = false;
        return;
    }

    // always call hover when we're overlapping
    onHover();

    if (mouseDown && wasPressed == false) onDown();
    else if (mouseDown == false && wasPressed) onUp();

    wasPressed = mouseDown;
}