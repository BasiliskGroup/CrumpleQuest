#ifndef PAPER_H
#define PAPER_H

#include "util/includes.h"
#include "levels/singleSide.h"

class Floor;

class Paper {
private:
    Floor* floor;
    std::pair<SingleSide*, SingleSide*> sides;
    SingleSide* curSide;
    bool isOpen;

public:
    Paper();
    Paper(Floor* floor, SingleSide* sideA, SingleSide* sideB, int startSide=0, bool isOpen=false);
    Paper(const Paper& other) ;
    Paper(Paper&& other);
    ~Paper();

    Paper& operator=(const Paper& other) noexcept;
    Paper& operator=(Paper&& other) noexcept;

    void flip();
    void open();

private:
    void clear();
};

#endif