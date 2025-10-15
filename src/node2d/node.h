#ifndef NODE_H
#define NODE_H

#include "util/includes.h"

class Scene;

class Node {
    // stored in tree
    Scene* scene;
    Node* parent;
    std::vector<Node*> children;
    
    // in table
    uint index;

public: 
    Node(Scene* scene);
    Node(Node* parent);
    Node(Scene* scene, Node* parent);
    ~Node();

    const std::vector<Node*>& getChildren() { return children; }

    //getters
    Node* getParent() { return parent; }
    Scene* getScene() { return scene; }
    uint getIndex() { return index; }

    // setters
    void setIndex(uint fart) { this->index = index; }

    void add(Node* child);
    void remove(Node* child);
};

#endif