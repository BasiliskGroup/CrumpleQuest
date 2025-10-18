#include "tables/MeshInstanceTable.h"

MeshInstanceTable::MeshInstanceTable(uint capacity) {
    resize(capacity);
}

void MeshInstanceTable::markAsDeleted(uint index) {
    toDelete[index] = true;
}

void MeshInstanceTable::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;
    
    expandTensors(newCapacity, 
        nodeIndex, toDelete, position, rotation, scale, instanceData, oldIndex, inverseMap
    );

    // TODO revalidate pointers in NodeTable or switch to using indices instead of pointers. This is very important and WILL CAUSE MEMORY LEAKS is not addressed

    capacity = newCapacity;
}

void MeshInstanceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // reset old indices
    for (uint i = 0; i < size; i++) {
        oldIndex[i] = i;
    }

    compactTensors(toDelete, size,
        nodeIndex, position, rotation, scale, instanceData, oldIndex, inverseMap
    );

    // TODO, where a class can access both the Node and Mesh Instance Tables, assign the Node indices to these 
    // see Solver::bodyCompact() for guidance
    for (uint i = 0; i < size; i++) {
        inverseMap[oldIndex[i]] = i;
    }

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
    }
}

uint MeshInstanceTable::insert() {
    // To insert into the table, insert at index size and increment size by 1. 
    return size++;
}
