#ifndef SCENE_H
#define SCENE_H

#include "solver/physics.h"

class Node;

class Scene {
    Solver* solver;
    Node* root;

public:
    Scene();
    ~Scene();

    Node* getRoot() { return root; }
};

#endif