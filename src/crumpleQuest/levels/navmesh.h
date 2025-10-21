#ifndef NAVMESH_H
#define NAVMESH_H

#include "util/includes.h"
#include "util/maths.h"

class Navmesh {
private:
    using Edge = const std::pair<const vec2&, const vec2&>;

    struct Triangle {
        Vec2Triplet verts;
        vec2 center;
        std::unordered_map<uint, ushort> adjacency;
        float g;
        float f;

        Triangle(Vec2Triplet verts);
        ~Triangle() = default;

        void reset();
        Edge operator[](size_t index) const;
        bool contains(const vec2& pos) const;
    };

    std::vector<vec2> mesh;
    std::vector<uint> rings; // contains the end index of all ring segments
    std::vector<Triangle> triangles;

public:
    Navmesh(const std::vector<vec2>& paperMesh);
    ~Navmesh() = default;
    
    uint addObstacle(std::vector<vec2> obstacleMesh);
    void earcut();
    void buildGraph();
    float heuristic(uint cur, uint goal);
    uint posToTriangle(const vec2& pos);
    void clearAStar();
    void AStar();
    void funnel();
    void clear();

};

#endif