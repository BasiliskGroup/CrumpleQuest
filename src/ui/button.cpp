#include "ui/ui.h"

Button::Button(Game* game, Node2D::Params params, Button::Params callbacks) : 
    Node2D(game->getScene(), params),
    onDown(callbacks.onDown),
    onUp(callbacks.onUp),
    onHover(callbacks.onHover)
{
    setLayer(0.9);
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

