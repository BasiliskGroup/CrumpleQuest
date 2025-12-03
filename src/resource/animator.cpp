#include "resource/animator.h"

void Animator::update() {
    if (time > timePerFrame) {
        time = 0.0;
        frame += 1;
    }
    if (frame >= animation->getNumberFrames()) {
        frame = 0;
    }

    time += engine->getDeltaTime();
    node->setMaterial(animation->getFrame(frame));
}

void Animator::setAnimation(Animation* animation) {
    if (animation == this->animation) { return; }
    this->animation = animation;
    time = 0.0;
    frame = 0;
}