#ifndef SLIDER_H
#define SLIDER_H

#include "util/includes.h"
#include "ui/button.h"

class Game;

class Slider : public UIElement {
private:
    struct Params {
        std::function<void(float)> callback = [](float proportion) {};

        // node stuff 
        Mesh* pegMesh = nullptr;
        Material* pegMaterial = nullptr;
        vec2 pegDim = {0.5, 0.5};
        float startProportion = 0.3;

        Material* barMaterial = nullptr;
        float barHeight = 0.25;
    };

    Node2D* bar;
    Button* peg;
    std::function<void(float)> callback;
    vec2 start;
    vec2 end;

    float startDot = 0;
    float endDot = 0;
    vec2 dir = { 0, 0 };
    bool holding = false;

    Game* game;

public:
    Slider(Game* game, const vec2& start, const vec2& end, Params params);
    ~Slider();
    
    // Rule of 5
    Slider(const Slider& other); 
    Slider& operator=(const Slider& other); 
    Slider(Slider&& other) noexcept; 
    Slider& operator=(Slider&& other) noexcept;

    void event(const vec2& mousePos, bool mouseDown) override;
    float getProportion();
    void setProportion(float proportion);
    void setCallback(std::function<void(float)> callback) { this->callback = callback; }

private:
    void computeBoundGeometry();
};

#endif