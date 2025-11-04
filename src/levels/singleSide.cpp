#include "levels/singleSide.h"

SingleSide::SingleSide() {
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
}

SingleSide& SingleSide::operator=(const SingleSide& other) noexcept {
    if (this == &other) return *this;
    clear();

    for (const Obstacle* obst : other.obstacles) {
        obstacles.push_back(new Obstacle(*obst));
    }

    navmesh = new Navmesh(*other.navmesh);
    enemies = other.enemies;

    return *this;
}

SingleSide& SingleSide::operator=(SingleSide&& other) noexcept {
    if (this == &other) return *this;
    clear();

    for (Obstacle* obst : other.obstacles) {
        obstacles.push_back(obst);
    }
    other.obstacles.clear(); // unlink all other pointers to prevent deletion

    navmesh = other.navmesh;
    enemies = std::move(other.enemies);

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
        Obstacle* obstacle = obstacles.back();
        obstacles.pop_back();
        delete obstacle;
    }
}