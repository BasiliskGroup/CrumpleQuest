#ifndef PAPER_H
#define PAPER_H

#include "util/includes.h"
#include "levels/singleSide.h"

class Paper {
public:
    static std::unordered_map<std::string, Paper> templates;

private:
    std::pair<SingleSide*, SingleSide*> sides;
    SingleSide* curSide;
    bool isOpen;

public:
    Paper();
    Paper(SingleSide* sideA, SingleSide* sideB, int startSide=0, bool isOpen=false);
    Paper(const Paper& other) noexcept;
    Paper(Paper&& other) noexcept;
    ~Paper();

    Paper& operator=(const Paper& other) noexcept;
    Paper& operator=(Paper&& other) noexcept;

    void flip();
    void open();

    static void generateTemplates(Game* game);

private:
    void clear();
};

#endif