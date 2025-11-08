#include "levels/navmesh.h"

Navmesh::Triangle::Triangle(vec2& a, vec2& b, vec2& c) : Tri({ a, b, c }) {
    center = (a + b + c) / 3.0f;
    reset();
}

void Navmesh::Triangle::reset() {
    g = 0;
    f = 0;
}

Navmesh::Edge Navmesh::Triangle::operator[](size_t index) const {
    return { verts[index], verts[(index + 1) % 3] };
}

Navmesh::Navmesh(const std::vector<vec2>& paperMesh) {
    mesh = paperMesh;
    rings.push_back(mesh.size());
}

uint Navmesh::addObstacle(std::vector<vec2> obstacleMesh) {
    mesh.insert(mesh.end(), obstacleMesh.rend(), obstacleMesh.rbegin());
    return mesh.size();
}

/**
 * @brief earcut mesh and initialize triangle array. Uses earcut.hpp
 * 
 */
void Navmesh::earcut() {
    std::vector<uint> indices;
    // Navmesh::earcutMesh(mesh, rings, indices);

    // construct triangles list
    for (uint i = 0; i < indices.size() / 3; i++) {
        triangles.emplace_back(
            mesh[indices[3 * i + 0]],
            mesh[indices[3 * i + 1]],
            mesh[indices[3 * i + 2]]
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

float Navmesh::heuristic(int cur, int dest){
    return heuristic(triangles[cur].center, triangles[dest].center);
}

int Navmesh::posToTriangle(const vec2& pos){
    for (int i = 0; i < triangles.size(); i++) {
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
    int start = posToTriangle(startPos);
    int dest = posToTriangle(destPos);
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

/**
 * @brief selects the edges of triangles connecting triangles along the A* path
 * 
 * @param path the indices of all triangles taken by A*
 */
void Navmesh::getPortals(const std::vector<uint>& path) {
    portals.clear();

    for (uint i = 0; i < path.size() - 1; i++) {
        const Triangle& triangle = triangles[path[i]];
        ushort edgeIndex = triangle.adjacency.at(path[i + 1]);
        portals.push_back(triangle[edgeIndex]);
    }
}

void Navmesh::funnel(const vec2& start, const vec2& dest, std::vector<vec2>& path) {
    path.clear();
    path.push_back(start);

    // if we are in the same triangle, take the direct path
    if (portals.size() == 0) {
        path.push_back(dest);
        return;
    }

    // initialize funnel
    uint apexIndex = 0;
    uint leftIndex = 0;
    uint rightIndex = 0;

    vec2 apex = start;
    vec2 left = portals[0].first;
    vec2 right = portals[0].second;

    uint i = 1;
    while (i < portals.size()) {
        vec2 newLeft = portals[i].first;
        vec2 newRight = portals[i].second;

        // update right
        if (sign(apex, right, newRight) <= 0) {
            if (glm::length2(apex - right) < EPSILON || sign(apex, left, newRight) > 0) {

                right = newRight;
                rightIndex = i;

            } else {

                // move apex left
                path.push_back(left);
                apex = left;
                apexIndex = leftIndex;
                left = right = apex;
                leftIndex = rightIndex = apexIndex;

                i = apexIndex + 1;
                continue;
            }
        }

        // update left
        if (sign(apex, left, newLeft) >= 0) {
            if (glm::length2(apex - left) < EPSILON || sign(apex, right, newLeft) < 0) {

                left = newLeft;
                leftIndex = i;

            } else {

                // apex moves right
                path.push_back(right);
                apex = right;
                apexIndex = rightIndex;
                left = right = apex;
                leftIndex = rightIndex = apexIndex;

                i = apexIndex + 1;
                continue;

            }
        }

        i += 1;
    }

    path.push_back(dest);
}

void Navmesh::generateNavmesh() {

}