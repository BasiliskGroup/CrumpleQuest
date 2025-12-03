#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "util/includes.h"
#include "resource/animation.h"

class Animator {
    private:
        bsk::Engine* engine;
        bsk::Node2D* node;
        Animation* animation;
        
        unsigned int frame = 0;
        float time = 0;
        float timePerFrame = 0.5;

    public:
        Animator(bsk::Engine* engine, bsk::Node2D* node, Animation* animation): engine(engine), node(node), animation(animation) {}

        void update();
        void setNode(Node2D* node) { this->node = node; }
        void setAnimation(Animation* animation);
        void setFrameRate(float frameRate) { timePerFrame = 1.0 / frameRate; }
};

#endif