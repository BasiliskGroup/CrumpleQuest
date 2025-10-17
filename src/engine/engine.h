#ifndef ENGINE_H
#define ENGINE_H

#include "physics.h"
#include "scene/basilisk.h"


class Engine {
private:
    Solver* solver; // TODO add the solver to the scene
    Scene* scene;

public:
    Engine();
    ~Engine();

    void update(float dt);
};

#endif