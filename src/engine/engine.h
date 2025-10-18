#ifndef ENGINE_H
#define ENGINE_H

#include "scene/basilisk.h"

class Engine {
private:
    Scene* scene;

public:
    Engine();
    ~Engine();

    void update(float dt);
};

#endif