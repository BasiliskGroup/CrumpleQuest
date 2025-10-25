#include "crumpleQuest/levels/floor.h"

Floor::Floor() {
    int distMax = FLOOR_WIDTH * FLOOR_WIDTH;

    for (uint x = 0; x < FLOOR_WIDTH; x++) {
        for (uint y = 0; y < FLOOR_WIDTH; y++) {
            playMap[x][y] = -1;
            tempMap[x][y] = 0;
            distMap[x][y] = distMax;
        }
    }
}

void Floor::generateFloor() {
    
}

// helper functions
bool Floor::inRange(const Position& pos) const {
    return 0 <= pos.x && pos.x < FLOOR_WIDTH && 0 <= pos.y && pos.y < FLOOR_WIDTH;
}

void Floor::getAround(const Position& pos, std::vector<Position>& around) const {
    around.clear();
    if (inRange({ pos.x - 1, pos.y })) around.emplace_back(pos.x - 1, pos.y);
    if (inRange({ pos.x + 1, pos.y })) around.emplace_back(pos.x + 1, pos.y);
    if (inRange({ pos.x, pos.y - 1 })) around.emplace_back(pos.x, pos.y - 1);
    if (inRange({ pos.x, pos.y + 1 })) around.emplace_back(pos.x, pos.y + 1);
}

void Floor::addToMaps(const Position& pos, int type) {
    playMap[pos.x][pos.y] = type;
    tempMap[pos.x][pos.y] = -1;

    std::vector<Position> around;
    getAround(pos, around);
    for (const auto& adj : around) {
        distMap[adj.x][adj.y] = glm::min(distMap[pos.x][pos.y], distMap[adj.x][adj.y] + 1);

        // we have already added this room
        if (tempMap[adj.x][adj.y] == -1) continue;

        tempMap[adj.x][adj.y] += 1;
        valids[adj] = tempMap[adj.x][adj.y];
    }
}