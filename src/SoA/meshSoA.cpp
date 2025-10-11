#include "SoA/meshSoA.h"
#include "util/print.h"

MeshSoA::MeshSoA(uint vertCapacity, uint meshCapacity) {
    this->vertCapacity = vertCapacity;
    this->meshCapacity = meshCapacity;

    // resize to meet vertsCapacity
    verts.resize(vertCapacity);

    // resize to meet meshCapacity
    halfDim.resize(meshCapacity);
    length.resize(meshCapacity);
    moment.resize(meshCapacity);
    start.resize(meshCapacity);
    area.resize(meshCapacity);
    com.resize(meshCapacity);
}

void MeshSoA::compact() {
    eraseChunks(verts, start, length, toDelete, meshes, halfDim, moment, area, com);
    toDelete.clear();
}

uint MeshSoA::insert(std::vector<vec2> verts) {    
    // check to see if the vector is going to resize
    bool needsResize = this->verts.capacity() < verts.size() + vertSize;
    
    // TODO center mesh com on (0, 0)
    for (uint i = 0; i < verts.size(); i++) {
        this->verts[vertSize + i] = verts[i];
    }

    if (needsResize) {
        refreshPointers();
    }

    start[meshSize] = vertSize;
    length[meshSize] = verts.size();

    // insert remaining property data
    // find extreme values
    float minX = INFINITY, minY = INFINITY;
    float maxX = -INFINITY, maxY = -INFINITY;
    
    for (uint i = 0; i < verts.size(); ++i) {
        vec2& v = verts[i];

        if (v.x < minX) minX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.x > maxX) maxX = v.x;
        if (v.y > maxY) maxY = v.y;
    }

    // calculate geometric center
    float gcx = (minX + maxX) / 2;
    float gcy = (minY + maxY) / 2;

    // calulate mesh variables TODO actually do this LATER. 
    // for now, default to unit cube values
    // create half dimensions
    halfDim[meshSize] = { maxX - gcx, maxY - gcy };

    // TODO add the follow variables 
    // com
    // area
    // moment

    // increment both sizes
    vertSize += verts.size();
    return meshSize++;
}

void MeshSoA::resize(uint new_capacity) {

}

void MeshSoA::refreshPointers() {
    for (const auto& pair : meshes) {
        pair.second->setIndex(pair.first);
        // TODO reroute pointer to correct starting location
    }
}

void MeshSoA::remove(uint meshIndex) {
    toDelete.push_back(meshIndex);
    meshes.erase(meshIndex);
}