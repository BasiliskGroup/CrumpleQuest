#ifndef FLOOR_H
#define FLOOR_H

#include "util/includes.h"
#include "util/random.h"
#include "levels/paper.h"

#define FLOOR_WIDTH 3
#define FLOOR_MIN_ROOMS 10
#define FLOOR_MEAN_ROOMS 13
#define FLOOR_STDEV_ROOMS 2
#define FLOOR_TEMP_REDUCT 0.92

class Paper;
class Game;

class Floor {
private:
    // store infomation about the grid
    template <typename T>
    struct SquareMap : std::array<std::array<T, FLOOR_WIDTH>, FLOOR_WIDTH> {
        using Base = std::array<std::array<T, FLOOR_WIDTH>, FLOOR_WIDTH>;
        using Base::operator[];
        using Base::Base;
    };

    // replacement for pair so I can use .x and .y
    struct Position {
        int x;
        int y;

        bool operator==(const Position& other) const noexcept {
            return x == other.x && y == other.y;
        }

        Position() : x(0), y(0) {}
        Position(int x, int y) : x(x), y(y) {}
    };

    struct PositionHash {
        std::size_t operator()(const Position& p) const noexcept {
            std::size_t h1 = std::hash<int>{}(p.x);
            std::size_t h2 = std::hash<int>{}(p.y);
            return h1 ^ (h2 << 1);
        }
    };

    // generation data
    int distMax = FLOOR_WIDTH * FLOOR_WIDTH;
    int center;
    std::unordered_map<Position, int, PositionHash> valids;
    SquareMap<RoomTypes> playMap;
    SquareMap<int> tempMap;
    SquareMap<int> distMap;

    // play data
    Position playerPos;
    SquareMap<Paper*> roomMap;
    Game* game;

public:
    Floor(Game* game);
    ~Floor();

    void getOptions(std::vector<Position> directions);
    Paper* getCenterRoom() const;
    Paper* getRoom(int x, int y) const;
    Paper* getCurrentRoom() const;
    Position getCurrentPosition() const { return playerPos; }
    int getCurrentX() const { return playerPos.x; }
    int getCurrentY() const { return playerPos.y; }
    Paper* getAdjacentRoom(int dx, int dy) const;
    void setCurrentPosition(int x, int y);
    RoomTypes getRoomType(int x, int y) const;

private:
    void generateFloor();
    void loadRooms();

    // helper functions
    bool inRange(const Position& pos) const;
    void getAround(const Position& pos, std::vector<Position>& around) const;
    void addToMaps(const Position& pos, RoomTypes type);
};

#endif