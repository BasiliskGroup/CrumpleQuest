#include "levels/levels.h"
#include "game/game.h"

Floor::Floor(Game* game) : roomMap(), game(game) {
    for (uint x = 0; x < FLOOR_WIDTH; x++) {
        for (uint y = 0; y < FLOOR_WIDTH; y++) {
            playMap[x][y] = NULL_ROOM;
            tempMap[x][y] = 0;
            distMap[x][y] = distMax;
            roomMap[x][y] = nullptr;
        }
    }

    center = FLOOR_WIDTH / 2;
    playerPos = { center, center };
    generateFloor();
    loadRooms();
}

void Floor::loadRooms() {
    for (uint x = 0; x < FLOOR_WIDTH; x++) {
        for (uint y = 0; y < FLOOR_WIDTH; y++) {
            if (playMap[x][y] == NULL_ROOM) continue;
            roomMap[x][y] = Paper::getRandomTemplate(playMap[x][y]);
            if (roomMap[x][y]) {
                roomMap[x][y]->setGame(game);
                roomMap[x][y]->regenerateWalls();
            }
        }
    }
}

void Floor::generateFloor() {
    // default values for starter room
    distMap[center][center] = 0;
    addToMaps({ center, center }, SPAWN_ROOM);

    // generate the remaining floor
    uint numRooms = glm::max(FLOOR_MIN_ROOMS, randomIntNormal(FLOOR_MEAN_ROOMS, FLOOR_STDEV_ROOMS));
    for (uint i = 1; i < numRooms; i++) {
        
        // Check if we have any valid positions
        if (valids.empty()) {
            break; // No more valid positions to add rooms
        }
        
        // sort dictionary by keys
        std::vector<std::pair<Position, int>> adjs(valids.begin(), valids.end());
        std::sort(adjs.begin(), adjs.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
        
        // Safety check
        if (adjs.empty()) {
            break;
        }
        
        Position pos = adjs[0].first;

        // get nth random based on temp decrease
        float prob = 1;
        uint j = 1;
        while (uniform(0, 1) < prob && j < adjs.size()) {
            pos = adjs[j].first;
            j++;
            prob *= FLOOR_TEMP_REDUCT;
        }

        // add the room to the maps
        valids.erase(pos);
        addToMaps(pos, BASIC_ROOM);
    }

    // add boss room to furthest valid location
    Position high = { center, center };
    for (int x = 0; x < FLOOR_WIDTH; x++) {
        for (int y = 0; y < FLOOR_WIDTH; y++) {
            if (distMap[x][y] == distMax or distMap[x][y] < distMap[high.x][high.y]) continue;
            high = { x, y };
        }
    }

    playMap[high.x][high.y] = BOSS_ROOM;
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

void Floor::addToMaps(const Position& pos, RoomTypes type) {
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

void Floor::getOptions(std::vector<Position> directions) {

}

Floor::~Floor() {
    // Clean up all Paper instances
    for (uint x = 0; x < FLOOR_WIDTH; x++) {
        for (uint y = 0; y < FLOOR_WIDTH; y++) {
            if (roomMap[x][y]) {
                delete roomMap[x][y];
                roomMap[x][y] = nullptr;
            }
        }
    }
}

Paper* Floor::getCenterRoom() const {
    return roomMap[center][center];
}

Paper* Floor::getRoom(int x, int y) const {
    if (x < 0 || x >= FLOOR_WIDTH || y < 0 || y >= FLOOR_WIDTH) {
        return nullptr;
    }
    return roomMap[x][y];
}

Paper* Floor::getCurrentRoom() const {
    return getRoom(playerPos.x, playerPos.y);
}

Paper* Floor::getAdjacentRoom(int dx, int dy) const {
    int newX = playerPos.x + dx;
    int newY = playerPos.y + dy;
    return getRoom(newX, newY);
}

void Floor::setCurrentPosition(int x, int y) {
    if (x >= 0 && x < FLOOR_WIDTH && y >= 0 && y < FLOOR_WIDTH) {
        playerPos = { x, y };
    }
}