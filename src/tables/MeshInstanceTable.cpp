#include "tables/MeshInstanceTable.h"

MeshInstanceTable::MeshInstanceTable(uint capacity) {
    resize(capacity);
}

void MeshInstanceTable::markAsDeleted(uint index) {
    toDelete[index] = true;
}

void MeshInstanceTable::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;
    expandTensors(size, newCapacity, 
        nodeIndex, toDelete, position, rotation, scale, instanceData
    );
    capacity = newCapacity;
}

void MeshInstanceTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    compactTensors(toDelete, size,
        nodeIndex, position, rotation, scale, instanceData
    );

    size = active;

    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
    }
}

uint MeshInstanceTable::insert() {
    // To insert into the table, insert at index size and increment size by 1. 
}
