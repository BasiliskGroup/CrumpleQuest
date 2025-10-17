#ifndef NODE_TABLE_H
#define NODE_TABLE_H

#include "tables/virtualTable.h"

class Node;
struct MeshInstanceData; // todo rename this

class NodeTable : public VirtualTable {
private:
    // compacting
    std::vector<bool> toDelete;
    std::vector<uint> oldIndex;
    std::vector<uint> inverseMap;

    // node object
    std::vector<Node*> nodes;

    // node data
    std::vector<uint> instanceIndex;
    std::vector<MeshInstanceData*> instanceTable;

public:
    NodeTable(uint capacity);
    ~NodeTable() = default;

    void markAsDeleted(uint index);

    auto& getNode() { return nodes; }
    auto& getInstanceIndex() { return instanceIndex; }
    auto& getInstanceTable() { return instanceTable; }

    void resize(uint newCapacity) override;
    void compact() override;
};

#endif