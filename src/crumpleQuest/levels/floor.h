#ifndef FLOOR_H
#define FLOOR_H

#include "util/includes.h"

#define FLOOR_WIDTH 7
#define FLOOR_MIN_ROOMS 10
#define FLOOR_MEAN_ROOMS 13
#define FLOOR_STDEV_ROOMS 2
#define FLOOR_TEMP_REDUCT 0.95

enum Rooms {
    SPAWN,
    BASIC,
    BOSS    
};

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

        Position(int x, int y) : x(x), y(y) {}
    };

    struct PositionHash {
        std::size_t operator()(const Position& p) const noexcept {
            std::size_t h1 = std::hash<int>{}(p.x);
            std::size_t h2 = std::hash<int>{}(p.y);
            return h1 ^ (h2 << 1);
        }
    };

    uint center;
    std::unordered_map<Position, int, PositionHash> valids;
    SquareMap<int> playMap;
    SquareMap<int> tempMap;
    SquareMap<int> distMap;

public:
    Floor();
    ~Floor() = default;

    void generateFloor();

private:
    // helper functions
    bool inRange(const Position& pos) const;
    void getAround(const Position& pos, std::vector<Position>& around) const;
    void addToMaps(const Position& pos, int type);
};

#endif