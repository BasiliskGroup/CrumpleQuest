#ifndef VIRTUAL_TABLE_H
#define VIRTUAL_TABLE_H

#include "util/includes.h"
#include "tables/eraseChunks.h"
#include "tables/expandTensor.h"
#include "tables/compactTensor.h"

class VirtualTable {
protected:
    uint size = 0;
    uint capacity = 0;

public:
    virtual void resize(uint new_capacity) = 0;
    virtual void compact() = 0;

    uint getSize() { return size; }
    uint getCapacity() { return capacity; } 
};

#endif