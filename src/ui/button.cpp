#include "ui/ui.h"

Button::Button(Scene2D* scene, Game* game, Node2D::Params params, Button::Params callbacks) : 
    Node2D(scene, params),
    onDown(callbacks.onDown),
    onUp(callbacks.onUp),
    onHover(callbacks.onHover),
    normalMaterial(params.material),
    hoverMaterial(callbacks.hoverMaterial),
    hitboxScale(callbacks.hitboxScale.x > 0 ? callbacks.hitboxScale : params.scale),
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
    isCurrentlyHovered(false),
    normalMaterial(other.normalMaterial),
    hoverMaterial(other.hoverMaterial),
    hitboxScale(other.hitboxScale),
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
        isCurrentlyHovered = false;
        normalMaterial = other.normalMaterial;
        hoverMaterial = other.hoverMaterial;
        hitboxScale = other.hitboxScale;
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
    isCurrentlyHovered(other.isCurrentlyHovered),
    normalMaterial(other.normalMaterial),
    hoverMaterial(other.hoverMaterial),
    hitboxScale(other.hitboxScale),
    game(other.game)
{
    // Reset other's state
    other.wasPressed = false;
    other.isCurrentlyHovered = false;
    other.normalMaterial = nullptr;
    other.hoverMaterial = nullptr;
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
        isCurrentlyHovered = other.isCurrentlyHovered;
        normalMaterial = other.normalMaterial;
        hoverMaterial = other.hoverMaterial;
        hitboxScale = other.hitboxScale;
        game = other.game;
        
        // Reset other's state
        other.wasPressed = false;
        other.isCurrentlyHovered = false;
        other.normalMaterial = nullptr;
        other.hoverMaterial = nullptr;
        other.game = nullptr;
    }
    return *this;
}

bool Button::isHovered(const vec2& pos) {
    // Scale is the full width/height (quad mesh goes from -0.5 to 0.5, so scale of 2.0 = 2.0 units wide)
    // So we need to use scale/2 to get the half-width/half-height for bounds checking
    float halfWidth = hitboxScale.x * 0.5f;
    float halfHeight = hitboxScale.y * 0.5f;
    return (this->position.x - halfWidth < pos.x && pos.x < this->position.x + halfWidth) 
        && (this->position.y - halfHeight < pos.y && pos.y < this->position.y + halfHeight);
}

void Button::event(const vec2& pos, bool mouseDown) {
    bool hovered = isHovered(pos);
    
    // Update hover state and material
    if (hovered && !isCurrentlyHovered) {
        isCurrentlyHovered = true;
        if (hoverMaterial) {
            this->setMaterial(hoverMaterial);
        }
    } else if (!hovered && isCurrentlyHovered) {
        isCurrentlyHovered = false;
        if (normalMaterial) {
            this->setMaterial(normalMaterial);
        }
    }
    
    if (!hovered) {
        wasPressed = false;
        return;
    }

    // always call hover when we're overlapping
    onHover();

    // Update state before calling callbacks to avoid use-after-free if callback destroys this button
    bool previousWasPressed = wasPressed;
    wasPressed = mouseDown;

    if (mouseDown && previousWasPressed == false) {
        onDown(); 
    }
    else if (mouseDown == false && previousWasPressed) {
        onUp();
    }
}