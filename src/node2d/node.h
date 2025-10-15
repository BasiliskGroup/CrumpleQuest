#ifndef NODE_H
#define NODE_H

#include "util/includes.h"

class Scene;

class Node {
    Scene* scene;
    Node* parent;
    std::vector<Node*> children;
    int fart = 0;

public: 
    Node(Scene* scene);
    Node(Node* parent);
    Node(Scene* scene, Node* parent);
    ~Node();

    const std::vector<Node*>& getChildren() { return children; }

    //getters
    Node* getParent() { return parent; }
    Scene* getScene() { return scene; }
    int getFart() { return fart; }

    // setters
    void setFart(int fart) { this->fart = fart; }

    void add(Node* child);
    void remove(Node* child);
};

#endif