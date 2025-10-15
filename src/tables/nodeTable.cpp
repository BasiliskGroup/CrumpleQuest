#include "scene/basilisk.h"

NodeTable::NodeTable(uint capacity) {
    resize(capacity);
}

void NodeTable::resize(uint newCapacity) {
    if (newCapacity <= capacity) return;

    expandTensors(size, newCapacity,
        nodes, toDelete, instanceIndex, instanceTable
    );

    capacity = newCapacity;
}

void NodeTable::compact() {
    // do a quick check to see if we need to run more complex compact function
    uint active = numValid(toDelete, size);
    if (active == size) {
        return;
    }

    // reset old indices TODO do this when we have mappings in
    // for (uint i = 0; i < size; i++) {
    //     oldIndex[i] = i;
    // }

    compactTensors(toDelete, size,
        nodes, instanceIndex, instanceTable
    );

    // update to current size
    size = active;

    // mark all active nodes as no delete and reset their index
    for (uint i = 0; i < size; i++) {
        toDelete[i] = false;
        nodes[i]->setIndex(i);
    }
}

void NodeTable::remove(uint index) {
    toDelete[index] = true;
}