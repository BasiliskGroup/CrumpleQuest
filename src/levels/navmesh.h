#ifndef NAVMESH_H
#define NAVMESH_H

#include "util/includes.h"
#include "util/maths.h"
#include "levels/triangle.h"

class Navmesh {
private:
    using Edge = std::pair<vec2, vec2>;

    struct Vec2Hash {
        std::size_t operator()(const vec2& v) const noexcept {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
        }
    };

    // Custom hash function for Edge
    struct EdgeHash {
        std::size_t operator()(const Edge& e) const noexcept {
            std::size_t h1 = Vec2Hash{}(e.first);
            std::size_t h2 = Vec2Hash{}(e.second);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    // Custom equality with epsilon tolerance for floating point comparison
    struct EdgeEqual {
        bool operator()(const Edge& a, const Edge& b) const noexcept {
            // Check if endpoints match (in either order) with tolerance
            bool match1 = glm::length2(a.first - b.first) < EPSILON * EPSILON &&
                         glm::length2(a.second - b.second) < EPSILON * EPSILON;
            bool match2 = glm::length2(a.first - b.second) < EPSILON * EPSILON &&
                         glm::length2(a.second - b.first) < EPSILON * EPSILON;
            return match1 || match2;
        }
    };

    struct Triangle : public Tri {
        vec2 center;
        std::unordered_map<uint, ushort> adjacency;
        float g;
        float f;

        Triangle(vec2& a, vec2& b, vec2& c);
        ~Triangle() = default;

        void reset();
        Edge operator[](size_t index) const;
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
    std::vector<Edge> portals;

public:
    Navmesh(const std::vector<vec2>& paperMesh);
    Navmesh(const Navmesh& other) = default;
    Navmesh(Navmesh&& other) = default;
    ~Navmesh() = default;

    Navmesh& operator=(const Navmesh& other) = default;
    Navmesh& operator=(Navmesh&& other) = default;
    
    void addObstacle(std::vector<vec2> obstacleMesh);
    void addMesh(std::vector<vec2> mesh);
    void getPath(std::vector<vec2>& path, vec2 start, vec2 dest);
    void generateNavmesh();

    static void earcut(const std::vector<std::vector<vec2>>& polygon, std::vector<uint>& indices);
    static void convertToMesh(const std::vector<std::vector<vec2>>& polygon, std::vector<uint>& indices, std::vector<float>& data);

    void clear();

private:
    void earcut();
    void buildGraph();

    float heuristic(int cur, int dest);
    float heuristic(const vec2& cur, const vec2& dest);

    int posToTriangle(const vec2& pos);
    void resetAlgoStructs();
    void AStar(const vec2& start, const vec2& dest, std::vector<uint>& path);
    void getPortals(const std::vector<uint>& path);
    void funnel(const vec2& start, const vec2& dest, std::vector<vec2>& path);

    inline float sign(const vec2& a, const vec2& b, const vec2& c) {
        vec2 ab = b - a;
        vec2 ac = c - a;
        return ab.x * ac.y - ab.y * ac.x;
    }
};

#endif