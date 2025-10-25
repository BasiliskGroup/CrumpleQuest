#include "crumpleQuest/levels/levels.h"

Paper::Paper() : floor(nullptr), curSide(nullptr), isOpen(false) {
    sides = { nullptr, nullptr };
}

/**
 * @brief Construct a new Paper:: Paper object
 * 
 * @param floor parent floor object
 * @param sideA A side of a paper, this needs to be unique to the paper
 * @param sideB A side of a paper, this needs to be unique to the paper
 * @param startSide 
 */
Paper::Paper(Floor* floor, SingleSide* sideA, SingleSide* sideB, int startSide, bool isOpen) : floor(floor), isOpen(isOpen) {
    this->floor = floor;
    sides = { sideA, sideB };
    curSide = startSide == 0 ? sideA : sideB;
}

Paper::Paper(const Paper& other) : floor(nullptr), curSide(nullptr), isOpen(false) {
    sides = { nullptr, nullptr };
    *this = other;
}

Paper::Paper(Paper&& other) : floor(nullptr), curSide(nullptr), isOpen(false) {
    sides = { nullptr, nullptr };
    *this = std::move(other);
}

Paper::~Paper() {
    clear();
}

Paper& Paper::operator=(const Paper& other) noexcept {
    if (this == &other) return *this;
    clear();

    floor = other.floor; // both have the same parent, not copy of parent

    if (other.sides.first != nullptr) {
        sides.first = new SingleSide(*other.sides.first);
    }

    if (other.sides.second != nullptr) {
        sides.second = new SingleSide(*other.sides.second);
    }
    
    if (other.curSide == nullptr) {
        curSide = nullptr;
    } else {
        curSide = other.curSide == other.sides.first ? sides.first : sides.second;
    }
    
    isOpen = other.isOpen;

    return *this;
}

Paper& Paper::operator=(Paper&& other) noexcept {
    if (this == &other) return *this;
    clear();

    floor = other.floor;
    sides = std::move(other.sides);
    curSide = std::move(other.curSide);
    isOpen = other.isOpen;

    other.clear();
    return *this;
}

void Paper::flip() {

}

void Paper::open() {

}

void Paper::clear() {
    floor = nullptr;
    delete sides.first;
    delete sides.second;
    sides = { nullptr, nullptr };
    curSide = nullptr;
}