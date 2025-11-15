#ifndef SLIDER_H
#define SLIDER_H

#include "util/includes.h"
#include "ui/button.h"

class Game;

class Slider {
private:
    struct Params {
        std::function<void(float proportion)> callback = [](float proportion) {};

        // node stuff 
    };

    Node2D* bar;
    Button* peg;
    std::function<void(float proportion)> callback;
    vec2 start;
    vec2 end;

    Game* game;

public:
    Slider(Game* game, const vec2& start, const vec2& end, Params params);
    ~Slider() = default;

    void event(const vec2& mousePos, bool mouseDown);
    float getProportion();
    float setProportion();
    void setCallback(std::function<void(float proportion)> callback) { this->callback = callback; }
};

#endif