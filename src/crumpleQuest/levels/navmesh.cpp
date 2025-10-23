#include "crumpleQuest/levels/navmesh.h"

Navmesh::Triangle::Triangle(vec2& a, vec2& b, vec2& c) {
    center = (a + b + c) / 3.0f;
    verts = { a, b, c };
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
    // TODO check if these are wound in the correct direction for the screen
    float d1 = sign(pos, verts[0], verts[1]); 
    float d2 = sign(pos, verts[1], verts[2]);
    float d3 = sign(pos, verts[2], verts[0]);

    bool hasNeg = d1 < 0 || d2 < 0 || d3 < 0;
    bool hasPos = d1 > 0 || d2 > 0 || d3 > 0;

    return !(hasNeg && hasPos);
}

Navmesh::Navmesh(const std::vector<vec2>& paperMesh) {
    mesh = paperMesh;
    rings.push_back(mesh.size());
}

uint Navmesh::addObstacle(std::vector<vec2> obstacleMesh) {
    mesh.insert(mesh.end(), obstacleMesh.rend(), obstacleMesh.rbegin());
    return mesh.size();
}

void Navmesh::earcut() {
    // convert into a format that works for earcut
    std::vector<std::vector<std::array<double, 2>>> polygon;
    size_t start = 0;
    for (auto end : rings) {
        std::vector<std::array<double, 2>> ring;
        for (size_t i = start; i < end; ++i)
            ring.push_back({ mesh[i].x, mesh[i].y });
        polygon.push_back(std::move(ring));
        start = end;
    }

    // earcut
    auto indices = mapbox::earcut<uint32_t>(polygon);

    // construct triangles list
    for (uint i = 0; i < indices.size() / 3; i++) {
        triangles.emplace_back(
            mesh[3 * i + 0],
            mesh[3 * i + 1],
            mesh[3 * i + 2]
        );
    }
}

/**
 * @brief Initializes the triangle-edge graph using the data stored about the individual triangles. Earcut must be run before calling this function. 
 * 
 */
void Navmesh::buildGraph(){
    std::unordered_map<Edge, std::pair<uint, ushort>, EdgeHash, EdgeEqual> edgeMap;

    for (uint trindex = 0; trindex < triangles.size(); trindex++) {
        Triangle& triangle = triangles[trindex];

        for (ushort edgindex = 0; edgindex < 3; edgindex++) {
            const Edge edge = triangle[edgindex];

            // determine if edge has a match using consistent winding
            Edge reverse = edge;
            std::swap(reverse.first, reverse.second);

            auto other = edgeMap.find(reverse);
            if (other == edgeMap.end()) {
                edgeMap[edge] = { trindex, edgindex };
                continue;
            }

            // we have found our match, pop and add to adjacency
            Triangle& otherTriangle = triangles[other->second.first];
            triangle.adjacency[other->second.first] = edgindex;
            otherTriangle.adjacency[trindex] = other->second.second;
            edgeMap.erase(other);
        }
    }
}

float Navmesh::heuristic(const vec2& cur, const vec2& dest) {
    return glm::length2(cur - dest);
}

float Navmesh::heuristic(uint cur, uint dest){
    return heuristic(triangles[cur].center, triangles[dest].center);
}

uint Navmesh::posToTriangle(const vec2& pos){
    for (uint i = 0; i < triangles.size(); i++) {
        if (triangles[i].contains(pos)) {
            return i;
        }
    }

    return -1;
}

void Navmesh::resetAlgoStructs(){
    open.clear();
    openLookup.clear();
    closed.clear();
    cameFrom.clear();
}

void Navmesh::clear() {
    resetAlgoStructs();
    mesh.clear();
    rings.clear();
    triangles.clear();
}

void Navmesh::getPath(std::vector<vec2>& path, vec2 start, vec2 dest) {
    resetAlgoStructs();

}

void Navmesh::AStar(const vec2& startPos, const vec2& destPos, std::vector<uint>& path) {
    // convert positions to triangles
    uint start = posToTriangle(startPos);
    uint dest = posToTriangle(destPos);
    path.clear();

    // check if we found valid pathing locations
    if (start < 0 || dest < 0) {
        return;
    }

    // prep values for initial triangle
    Triangle& startTri = triangles[start];
    startTri.g = 0;
    startTri.f = heuristic(start, dest);

    // add to data structures
    open.emplace(start, startTri.f);
    openLookup.insert(start);

    while (open.empty() == false) {
        uint curIdx = open.top().index;
        open.pop();

        Triangle& cur = triangles[curIdx];
        openLookup.erase(curIdx);

        // check if we have found the goal, reconstruct path
        if (curIdx == dest) {
            path.push_back(curIdx);
            while (cameFrom.find(curIdx) != cameFrom.end()) {
                curIdx = cameFrom[curIdx];
                path.push_back(curIdx);
            }
            std::reverse(path.begin(), path.end());
            return;
        }

        // mark current as visited
        closed.insert(curIdx);

        // integrate adjacents
        for (const auto& [adjIdx, edgeIdx] : cur.adjacency) {
            if (closed.find(adjIdx) != closed.end()) {
                continue;
            }

            // compute new neighbor cost
            Triangle& adj = triangles[adjIdx];
            float gTent = cur.g + heuristic(cur.center, adj.center);

            // if neighbor not in open set or find a better path
            bool notInLookup = openLookup.find(adjIdx) == openLookup.end();
            if (gTent < adj.g || notInLookup) {
                cameFrom[adjIdx] = curIdx;
                adj.g = gTent;
                adj.f = gTent + heuristic(adjIdx, dest);

                if (notInLookup) {
                    open.emplace(adjIdx, adj.f);
                    openLookup.insert(adjIdx);
                }
            }
        }
    }

    // no path found
    path.clear();
}

void Navmesh::funnel() {

}