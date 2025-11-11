#ifndef FLOOR_H
#define FLOOR_H

#include "util/includes.h"
#include "util/random.h"
#include "levels/paper.h"

#define FLOOR_WIDTH 7
#define FLOOR_MIN_ROOMS 10
#define FLOOR_MEAN_ROOMS 13
#define FLOOR_STDEV_ROOMS 2
#define FLOOR_TEMP_REDUCT 0.95

class Paper;

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

public:
    Floor();
    ~Floor() = default;

    void getOptions(std::vector<Position> directions);

private:
    void generateFloor();
    void loadRooms();

    // helper functions
    bool inRange(const Position& pos) const;
    void getAround(const Position& pos, std::vector<Position>& around) const;
    void addToMaps(const Position& pos, RoomTypes type);
};

#endif