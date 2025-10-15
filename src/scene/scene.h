#ifndef SCENE_H
#define SCENE_H

class Node;

class Scene {
    Node* root;

public:
    Scene();
    ~Scene();

    Node* getRoot() { return root; }
};

#endif