#ifndef NODETABLE_H
#define NODETABLE_H

#include "tables/virtualSoA.h"

class Node;
struct Data; // todo rename this

class NodeTable : public SoA {
private:
    std::vector<Node*> nodes;
    std::vector<uint> instanceIndex;
    std::vector<Data*> instanceTable;
    std::vector<bool> toDelete;

public:
    NodeTable(uint capacity);
    ~NodeTable() = default;

    auto& getNode() { return nodes; }
    auto& getInstanceIndex() { return instanceIndex; }
    auto& getInstanceTable() { return instanceTable; }

    void resize(uint newCapacity) override;
    void compact() override;
    void remove(uint index);
};

#endif