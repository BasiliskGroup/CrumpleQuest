#include "tables/colliderFlat.h"
#include "util/print.h"

ColliderFlat::ColliderFlat(uint vertCapacity, uint colliderCapacity) {
    this->vertCapacity = vertCapacity;
    this->colliderCapacity = colliderCapacity;

    // resize to meet vertsCapacity
    verts.resize(vertCapacity);

    // resize to meet colliderCapacity
    halfDim.resize(colliderCapacity);
    length.resize(colliderCapacity);
    moment.resize(colliderCapacity);
    start.resize(colliderCapacity);
    area.resize(colliderCapacity);
    com.resize(colliderCapacity);
}

void ColliderFlat::compact() {
    eraseChunks(verts, start, length, toDelete, colliders, halfDim, moment, area, com);
    toDelete.clear();
}

uint ColliderFlat::insert(std::vector<vec2> verts) {    
    // check to see if the vector is going to resize
    bool needsResize = this->verts.capacity() < verts.size() + vertSize;
    
    // TODO center collider com on (0, 0)
    for (uint i = 0; i < verts.size(); i++) {
        this->verts[vertSize + i] = verts[i];
    }

    if (needsResize) {
        refreshPointers();
    }

    start[colliderSize] = vertSize;
    length[colliderSize] = verts.size();

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
    halfDim[colliderSize] = { maxX - gcx, maxY - gcy };

    // TODO add the follow variables 
    // com
    // area
    // moment

    // increment both sizes
    vertSize += verts.size();
    return colliderSize++;
}

void ColliderFlat::resize(uint newCapacity) {
    
}

void ColliderFlat::refreshPointers() {
    for (const auto& pair : colliders) {
        pair.second->setIndex(pair.first);
        // TODO reroute pointer to correct starting location
    }
}

void ColliderFlat::remove(uint colliderIndex) {
    toDelete.push_back(colliderIndex);
    colliders.erase(colliderIndex);
}