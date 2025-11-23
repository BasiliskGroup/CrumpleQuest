#ifndef ANIMATION_H 
#define ANIMATION_H

#include "util/includes.h"

class Animation {
    private:
        std::vector<bsk::Material*> frames;
        
    public:
        Animation(std::vector<bsk::Material*> frames): frames(frames) {}

        unsigned int getNumberFrames() { return frames.size(); }
        bsk::Material* getFrame(unsigned int frame) { return frames.at(frame); }
};

#endif