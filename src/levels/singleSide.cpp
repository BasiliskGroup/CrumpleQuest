#include "levels/levels.h"

SingleSide::SingleSide() : navmesh(nullptr) {
    std::vector<vec2> mesh = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
    navmesh = new Navmesh(mesh);
}

SingleSide::SingleSide(const SingleSide& other) : navmesh(nullptr) {
    *this = other;
}

SingleSide::SingleSide(SingleSide&& other) : navmesh(nullptr) {
    *this = std::move(other);
}

SingleSide::~SingleSide() {
    delete navmesh;
    navmesh = nullptr;
}

SingleSide& SingleSide::operator=(const SingleSide& other) noexcept {
    if (this == &other) return *this;
    clear();

    for (const Node2D* node : other.obstacles) {
        obstacles.push_back(new Node2D(*node));
    }

    navmesh = new Navmesh(*other.navmesh);
    enemies = other.enemies;

    return *this;
}

SingleSide& SingleSide::operator=(SingleSide&& other) noexcept {
    if (this == &other) return *this;
    clear();

    for (Node2D* obst : other.obstacles) {
        obstacles.push_back(obst);
    }
    other.obstacles.clear(); // unlink all other pointers to prevent deletion

    navmesh = other.navmesh;
    enemies = std::move(other.enemies);

    other.navmesh = nullptr;
    other.clear();
    return *this;
}

void SingleSide::generateNavmesh() {
    navmesh->generateNavmesh();
}

void SingleSide::update(float dt) {

}

void SingleSide::clear() {
    delete navmesh;
    navmesh = nullptr;

    while (obstacles.empty() == false) {
        Node2D* node = obstacles.back();
        obstacles.pop_back();
        delete node;
    }
}