#ifndef VIRTUALSOA_H
#define VIRTUALSOA_H

#include "util/includes.h"
#include "tables/helper.h"
#include "tables/expandTensor.h"
#include "tables/compactTensor.h"

class SoA {
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