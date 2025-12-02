#include "levels/navmesh.h"
#include <earcut.hpp>

Navmesh::Triangle::Triangle(vec2& a, vec2& b, vec2& c) : Tri({ a, b, c }) {
    center = (a + b + c) / 3.0f;
    reset();
}

void Navmesh::Triangle::reset() {
    g = std::numeric_limits<float>::infinity();
    f = std::numeric_limits<float>::infinity();
}

Navmesh::Edge Navmesh::Triangle::operator[](size_t index) const {
    return { verts[index].pos, verts[(index + 1) % 3].pos };
}

Navmesh::Navmesh(const std::vector<vec2>& paperMesh) {
    mesh = paperMesh;
    rings.push_back(mesh.size());
}

void Navmesh::addMesh(std::vector<vec2> addedMesh) {
    mesh.insert(mesh.end(), addedMesh.begin(), addedMesh.end());
    rings.push_back(mesh.size());
}

void Navmesh::addObstacle(std::vector<vec2> obstacleMesh) {
    // Obstacles need to be reversed for proper winding
    mesh.insert(mesh.end(), obstacleMesh.rbegin(), obstacleMesh.rend());
    rings.push_back(mesh.size());
}

/**
 * @brief earcut mesh and initialize triangle array. Uses earcut.hpp
 */
void Navmesh::earcut() {
    std::vector<uint> indices;
    
    // Use earcut with mapbox format
    using Point = std::array<float, 2>;
    std::vector<std::vector<Point>> polygonData;
    
    uint startIdx = 0;
    for (uint ringEnd : rings) {
        std::vector<Point> ringData;
        for (uint i = startIdx; i < ringEnd; i++) {
            ringData.push_back({mesh[i].x, mesh[i].y});
        }
        if (!ringData.empty()) {
            polygonData.push_back(ringData);
        }
        startIdx = ringEnd;
    }
    
    indices = mapbox::earcut<uint>(polygonData);

    // Construct triangles list
    triangles.clear();
    for (uint i = 0; i < indices.size() / 3; i++) {
        triangles.emplace_back(
            mesh[indices[3 * i + 0]],
            mesh[indices[3 * i + 1]],
            mesh[indices[3 * i + 2]]
        );
    }
}

/**
 * @brief Initializes the triangle-edge graph using the data stored about the individual triangles. 
 * Earcut must be run before calling this function.
 */
void Navmesh::buildGraph() {
    std::unordered_map<Edge, std::pair<uint, ushort>, EdgeHash, EdgeEqual> edgeMap;

    for (uint trindex = 0; trindex < triangles.size(); trindex++) {
        Triangle& triangle = triangles[trindex];
        triangle.adjacency.clear();

        for (ushort edgindex = 0; edgindex < 3; edgindex++) {
            const Edge edge = triangle[edgindex];

            // Determine if edge has a match using consistent winding
            Edge reverse = edge;
            std::swap(reverse.first, reverse.second);

            auto other = edgeMap.find(reverse);
            if (other == edgeMap.end()) {
                edgeMap[edge] = { trindex, edgindex };
                continue;
            }

            // We have found our match, pop and add to adjacency
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

float Navmesh::heuristic(int cur, int dest) {
    return heuristic(triangles[cur].center, triangles[dest].center);
}

int Navmesh::posToTriangle(const vec2& pos) {
    for (int i = 0; i < triangles.size(); i++) {
        float dist = triangles[i].distance(pos);
        if (dist < 1e-8f) {
            return i;
        }
    }
    
    // Find closest triangle for debugging
    if (!triangles.empty()) {
        int closestIdx = 0;
        float closestDist = triangles[0].distance(pos);
        for (int i = 1; i < triangles.size(); i++) {
            float dist = triangles[i].distance(pos);
            if (dist < closestDist) {
                closestDist = dist;
                closestIdx = i;
            }
        }
        std::cout << "[Navmesh::posToTriangle] Position (" << pos.x << ", " << pos.y 
                  << ") not in any triangle. Closest triangle " << closestIdx 
                  << " at distance " << closestDist << std::endl;
    } else {
        std::cout << "[Navmesh::posToTriangle] Position (" << pos.x << ", " << pos.y 
                  << ") - no triangles exist!" << std::endl;
    }
    
    return -1;
}

void Navmesh::resetAlgoStructs() {
    open.clear();
    openLookup.clear();
    closed.clear();
    cameFrom.clear();
    
    // Reset all triangle pathfinding data
    for (auto& tri : triangles) {
        tri.reset();
    }
}

void Navmesh::clear() {
    resetAlgoStructs();
    mesh.clear();
    rings.clear();  // This clears rings
    triangles.clear();
    
    std::cout << "[Navmesh::clear] Cleared all data" << std::endl;
}

void Navmesh::getPath(std::vector<vec2>& path, vec2 start, vec2 dest) {
    path.clear();
    
    // Reset all algorithm structures to ensure clean state
    resetAlgoStructs();
    
    // Run A* to get triangle path
    std::vector<uint> trianglePath;
    AStar(start, dest, trianglePath);
    
    if (trianglePath.empty()) {
        return; // No path found
    }
    
    // Get portals from triangle path
    getPortals(trianglePath);
    
    // Run funnel algorithm to smooth path
    funnel(start, dest, path);
}

void Navmesh::AStar(const vec2& startPos, const vec2& destPos, std::vector<uint>& path) {
    // Convert positions to triangles
    int start = posToTriangle(startPos);
    int dest = posToTriangle(destPos);
    path.clear();

    // Check if we found valid pathing locations
    if (start < 0 || dest < 0) {
        return;
    }

    // Same triangle optimization
    if (start == dest) {
        path.push_back(start);
        return;
    }

    // Prep values for initial triangle
    Triangle& startTri = triangles[start];
    startTri.g = 0;
    startTri.f = heuristic(start, dest);

    // Add to data structures
    open.emplace(start, startTri.f);
    openLookup.insert(start);

    while (!open.empty()) {
        uint curIdx = open.top().index;
        open.pop();

        Triangle& cur = triangles[curIdx];
        openLookup.erase(curIdx);

        // Check if we have found the goal, reconstruct path
        if (curIdx == dest) {
            path.push_back(curIdx);
            while (cameFrom.find(curIdx) != cameFrom.end()) {
                curIdx = cameFrom[curIdx];
                path.push_back(curIdx);
            }
            std::reverse(path.begin(), path.end());
            return;
        }

        // Mark current as visited
        closed.insert(curIdx);

        // Integrate adjacents
        for (const auto& [adjIdx, edgeIdx] : cur.adjacency) {
            if (closed.find(adjIdx) != closed.end()) {
                continue;
            }

            // Compute new neighbor cost
            Triangle& adj = triangles[adjIdx];
            float gTent = cur.g + glm::length(cur.center - adj.center);

            // If neighbor not in open set or find a better path
            bool notInLookup = openLookup.find(adjIdx) == openLookup.end();
            if (gTent < adj.g) {
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

    // No path found
    path.clear();
}

/**
 * @brief Selects the edges of triangles connecting triangles along the A* path
 * @param path the indices of all triangles taken by A*
 */
void Navmesh::getPortals(const std::vector<uint>& path) {
    portals.clear();

    for (uint i = 0; i < path.size() - 1; i++) {
        const Triangle& triangle = triangles[path[i]];
        const Triangle& nextTriangle = triangles[path[i + 1]];
        
        auto it = triangle.adjacency.find(path[i + 1]);
        if (it != triangle.adjacency.end()) {
            ushort edgeIndex = it->second;
            Edge edge = triangle[edgeIndex];
            
            // Determine left/right based on movement direction through the corridor
            // When moving from triangle.center toward nextTriangle.center:
            // - If edge.first is to the LEFT of the movement direction, keep order
            // - If edge.first is to the RIGHT of the movement direction, swap
            
            vec2 moveDir = nextTriangle.center - triangle.center;
            vec2 edgeDir = edge.second - edge.first;
            
            // Cross product: positive means edgeDir is counter-clockwise from moveDir
            // This means edge goes from right to left, so we need to swap
            float cross = edgeDir.x * moveDir.y - edgeDir.y * moveDir.x;
            
            if (cross < 0) {
                // Edge goes left-to-right, keep it
                portals.push_back({edge.first, edge.second});
            } else {
                // Edge goes right-to-left, swap it
                portals.push_back({edge.second, edge.first});
            }
        }
    }
}

void Navmesh::funnel(const vec2& start, const vec2& dest, std::vector<vec2>& path) {
    path.clear();
    path.push_back(start);

    // If we are in the same triangle, take the direct path
    if (portals.empty()) {
        path.push_back(dest);
        return;
    }

    // Initialize funnel - SWAP left and right from portal notation
    uint apexIndex = 0;
    uint leftIndex = 0;
    uint rightIndex = 0;

    vec2 apex = start;
    vec2 left = portals[0].second;
    vec2 right = portals[0].first;

    uint i = 1;
    while (i < portals.size()) {
        vec2 newLeft = portals[i].second;
        vec2 newRight = portals[i].first;

        // Update right
        if (sign(apex, right, newRight) <= 0) {
            if (glm::length2(apex - right) < 1e-6f || sign(apex, left, newRight) > 0) {
                right = newRight;
                rightIndex = i;
            } else {
                // Move apex left
                path.push_back(left);
                apex = left;
                apexIndex = leftIndex;
                left = right = apex;
                leftIndex = rightIndex = apexIndex;
                i = apexIndex + 1;
                continue;
            }
        }

        // Update left
        if (sign(apex, left, newLeft) >= 0) {
            if (glm::length2(apex - left) < 1e-6f || sign(apex, right, newLeft) < 0) {
                left = newLeft;
                leftIndex = i;
            } else {
                // Apex moves right
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
    if (mesh.empty()) return;
    
    // Earcut the mesh
    earcut();
    
    if (triangles.empty()) return;
    
    // Build adjacency graph
    buildGraph();
}