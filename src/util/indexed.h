#ifndef INDEXED_H
#define INDEXED_H

#include "util/includes.h"

class Indexed {
protected:
    uint index;
public:
    uint getIndex() { return index; }
    void setIndex(uint index) { this->index = index; }
};

#endif