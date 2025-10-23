#ifndef NAVMESH_H
#define NAVMESH_H

#include "util/includes.h"
#include "util/maths.h"

class Navmesh {
private:
    using Edge = std::pair<vec2, vec2>;

    // Custom hash function for Edge
    struct EdgeHash {
        std::size_t operator()(const Edge& e) const noexcept {
            std::hash<glm::vec2> vec2Hash;
            // combine both vector hashes (standard hash combine pattern)
            std::size_t h1 = vec2Hash(e.first);
            std::size_t h2 = vec2Hash(e.second);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    // Optional: custom equality (only needed if float comparison tolerance required)
    struct EdgeEqual {
        bool operator()(const Edge& a, const Edge& b) const noexcept {
            return a.first == b.first && a.second == b.second;
        }
    };

    struct Triangle {
        uint index;
        Vec2Triplet verts;
        vec2 center;
        std::unordered_map<uint, ushort> adjacency;
        float g;
        float f;

        Triangle(Vec2Triplet verts, uint index);
        ~Triangle() = default;

        void reset();
        Edge operator[](size_t index) const;
        bool contains(const vec2& pos) const;
    };

    // nav mesh variables (not algo)
    std::vector<vec2> mesh;
    std::vector<uint> rings; // contains the end index of all ring segments
    std::vector<Triangle> triangles;

    // custom struct to hold pq triangle relationships
    struct PQPair {
        uint index;
        float f;

        PQPair(uint index, float f) : index(index), f(f) {}
    };

    // custom comparator for open set priority queue
    struct PQCompare {
        bool operator()(const PQPair& a, const PQPair& b) const {
            return a.f >b.f;
        }
    };

    // custom priority queue that support clearing
    class OpenQueue : public std::priority_queue<PQPair, std::vector<PQPair>, PQCompare> {
    public:
        void clear() { this->c.clear(); }
    };

    // a* variables
    OpenQueue open; 
    std::set<uint> openLookup;
    std::set<uint> closed;
    std::unordered_map<uint, uint> cameFrom;

    // funnel variables
    std::vector<Vec2Pair> portals;

public:
    Navmesh(const std::vector<vec2>& paperMesh);
    ~Navmesh() = default;
    
    uint addObstacle(std::vector<vec2> obstacleMesh);
    void getPath(std::vector<vec2>& path, vec2 start, vec2 dest);

private:
    void earcut();
    void buildGraph();

    float heuristic(uint cur, uint dest);
    float heuristic(const vec2& cur, const vec2& dest);

    uint posToTriangle(const vec2& pos);
    void resetAlgoStructs();
    void AStar(const vec2& start, const vec2& dest, std::vector<uint>& path);
    void funnel();
    void clear();

};

#endif