#include "scene/basilisk.h"
#include "util/print.h"


Node::Node(Scene* scene) : scene(scene), parent(scene->getRoot()) {
    parent->children.push_back(this);
}

// NOTE parent cannot be nullptr, this in intended
Node::Node(Node* parent) : scene(parent->getScene()), parent(parent) {
    parent->children.push_back(this);
}

// NOTE parent may be nullptr, this constructor should only be for Scene::root
Node::Node(Scene* scene, Node* parent) : scene(scene), parent(parent) {
    if (parent != nullptr) {
        parent->children.push_back(this);
    }
}

Node::~Node() {
    while (children.size() > 0) {
        Node* child = children.back();
        delete child;
    }

    if (parent != nullptr) {
        parent->remove(this);
    }
}

void Node::add(Node* child) {
    child->parent->remove(child);

    child->parent = this;
    children.push_back(child);
}

void Node::remove(Node* child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        children.erase(it);
    } else {
        throw std::runtime_error("could not find child");
    }
}