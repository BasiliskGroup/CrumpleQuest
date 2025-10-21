#include "crumpleQuest/levels/navmesh.h"

Navmesh::Triangle::Triangle(Vec2Triplet verts) {
    center = (verts[0] + verts[1] + verts[2]) / 3.0f;
    reset();
}

void Navmesh::Triangle::reset() {
    g = 0;
    f = 0;
}

Navmesh::Edge Navmesh::Triangle::operator[](size_t index) const {
    return { verts[index], verts[(index + 1) % 3] };
}

bool Navmesh::Triangle::contains(const vec2& pos) const {
    float d1 = triangleArea2(pos, verts[0], verts[1]);
    float d2 = triangleArea2(pos, verts[1], verts[2]);
    float d3 = triangleArea2(pos, verts[2], verts[0]);

    bool hasNeg = d1 < 0 || d2 < 0 || d3 < 0;
    bool hasPos = d1 > 0 || d2 > 0 || d3 > 0;

    return !(hasNeg && hasPos);
}

Navmesh::Navmesh(const std::vector<vec2>& paperMesh) {
    mesh = paperMesh;
    rings.push_back(mesh.size());
}

uint Navmesh::addObstacle(std::vector<vec2> obstacleMesh) {

    return -1;
}

void Navmesh::earcut() {

}

void Navmesh::buildGraph(){

}

float Navmesh::heuristic(uint cur, uint goal){
    return glm::length2(triangles[cur].center - triangles[goal].center);
}

uint Navmesh::posToTriangle(const vec2& pos){
    for (uint i = 0; i < triangles.size(); i++) {
        if (triangles[i].contains(pos)) {
            return i;
        }
    }

    return -1;
}

void Navmesh::clearAStar(){

}

void Navmesh::clear() {
    mesh.clear();
    rings.clear();
    triangles.clear();
}
