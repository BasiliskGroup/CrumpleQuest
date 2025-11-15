#ifndef BUTTON_H
#define BUTTON_H

#include "util/includes.h"

class Game;

class Button : public Node2D {
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

    bool isHovered(const vec2& pos);
    void event(const vec2& pos, bool mouseDown);

    void setOnDown(std::function<void()> onDown) { this->onDown = onDown; }
    void setOnUp(std::function<void()> onUp) { this->onUp = onUp; }
    void setOnHover(std::function<void()> onHover) { this->onHover = onHover; }
};

#endif