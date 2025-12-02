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

    // std::cout << "Frame : " << frame << " out of " << animation->getNumberFrames() << std::endl;
    node->setMaterial(animation->getFrame(frame));
}

void Animator::setAnimation(Animation* animation) {
    this->animation = animation;
    time = 0.0;
    frame = 0;
}