#include "levels/levels.h"

SingleSide::SingleSide() : navmesh(nullptr) {
    std::vector<vec2> mesh = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};
    navmesh = new Navmesh(mesh);
}

SingleSide::SingleSide(const SingleSide& other) noexcept : navmesh(nullptr) {
    if (other.navmesh) navmesh = new Navmesh(*other.navmesh);

    for (const Node2D* node : other.obstacles) {
        obstacles.push_back(new Node2D(*node));
    }

    enemies = other.enemies;
}

SingleSide::SingleSide(SingleSide&& other) noexcept : 
    navmesh(other.navmesh),
    obstacles(std::move(other.obstacles)),
    enemies(std::move(other.enemies))
{
    other.navmesh = nullptr;
    other.obstacles.clear();
}

SingleSide::~SingleSide() {
    delete navmesh;
    navmesh = nullptr;

    for (Node2D* node : obstacles) delete node;
    obstacles.clear();
}

SingleSide& SingleSide::operator=(const SingleSide& other) noexcept {
    if (this == &other) return *this;
    clear();

    if (other.navmesh) navmesh = new Navmesh(*other.navmesh);

    for (const Node2D* node : other.obstacles) {
        obstacles.push_back(new Node2D(*node));
    }

    enemies = other.enemies;
    return *this;
}

SingleSide& SingleSide::operator=(SingleSide&& other) noexcept {
    if (this == &other) return *this;
    clear();

    // transfer
    navmesh = other.navmesh;
    obstacles = std::move(other.obstacles); // vector of pointers transferred
    enemies = std::move(other.enemies);

    // clear other
    other.navmesh = nullptr;
    other.obstacles.clear();

    return *this;
}

void SingleSide::generateNavmesh() {
    if (navmesh) navmesh->generateNavmesh();
}

void SingleSide::update(float dt) {

}

void SingleSide::clear() {
    delete navmesh;
    navmesh = nullptr;

    for (Node2D* n : obstacles) delete n;
    obstacles.clear();
}