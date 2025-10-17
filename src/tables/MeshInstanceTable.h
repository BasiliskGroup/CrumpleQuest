#ifndef MESH_INSTANCE_H
#define MESH_INSTANCE_H

#include "tables/virtualTable.h"

class Node;

struct MeshInstanceData {
    mat4x4 model;
    uint material;
};

class MeshInstanceTable : VirtualTable {
private:
    // table management
    std::vector<uint> nodeIndex;
    std::vector<bool> toDelete;

    // jonah data
    std::vector<vec3> position;
    std::vector<quat> rotation;
    std::vector<vec3> scale;
    std::vector<MeshInstanceData> instanceData;

public:
    MeshInstanceTable(uint capacity);
    ~MeshInstanceTable() = default;

    void markAsDeleted(uint index);

    auto& getNodeIndex() { return nodeIndex; }
    auto& getPosition() { return position; }
    auto& getRotation() { return rotation; }
    auto& getScale() { return scale; }
    auto& getInstanceData() { return instanceData; }

    void resize(uint newCapacity) override;
    void compact() override;
    uint insert();
};

#endif