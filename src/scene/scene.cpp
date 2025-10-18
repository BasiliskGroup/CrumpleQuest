#include "scene/basilisk.h"

Scene::Scene() {
    root = new Node(this, nullptr);
}

Scene::~Scene() {
    delete root;
    delete solver;
}