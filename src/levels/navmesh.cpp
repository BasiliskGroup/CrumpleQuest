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
    // Try with tighter epsilon first (point is inside triangle)
    for (int i = 0; i < triangles.size(); i++) {
        float dist = triangles[i].distance(pos);
        if (dist < 1e-6f) {
            return i;
        }
    }
    
    // Find closest triangle for tolerance fallback
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
        
        // If very close, use it anyway (tolerance for floating point errors)
        if (closestDist < 0.01f) {
            return closestIdx;
        }
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
    rings.clear();
    triangles.clear();
}

void Navmesh::getPath(std::vector<vec2>& path, vec2 start, vec2 dest, float padding) {
    path.clear();
    
    // Reset all algorithm structures to ensure clean state
    resetAlgoStructs();
    
    // Run A* to get triangle path
    std::vector<uint> trianglePath;
    AStar(start, dest, trianglePath);
    
    if (trianglePath.empty()) {
        return; // No path found
    }
    
    // Get portals from triangle path with padding
    getPortals(trianglePath, padding);
    
    // Build path using portal centers as waypoints
    path.clear();
    path.push_back(start);
    
    // Add center of each portal as a waypoint
    for (const Edge& portal : portals) {
        vec2 portalCenter = (portal.first + portal.second) * 0.5f;
        path.push_back(portalCenter);
    }
    
    // Always add destination
    if (path.empty() || glm::length2(path.back() - dest) > 1e-6f) {
        path.push_back(dest);
    }
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
void Navmesh::getPortals(const std::vector<uint>& path, float padding) {
    portals.clear();

    for (uint i = 0; i < path.size() - 1; i++) {
        const Triangle& triangle = triangles[path[i]];
        const Triangle& nextTriangle = triangles[path[i + 1]];
        
        auto it = triangle.adjacency.find(path[i + 1]);
        if (it != triangle.adjacency.end()) {
            ushort edgeIndex = it->second;
            Edge edge = triangle[edgeIndex];
            
            vec2 portalLeft = edge.first;
            vec2 portalRight = edge.second;
            
            // Apply padding to push portal outward if padding > 0
            if (padding > 0.0f) {
                // Calculate portal edge direction
                vec2 edgeDir = portalRight - portalLeft;
                float edgeLen = glm::length(edgeDir);
                
                if (edgeLen > 1e-6f) {
                    // Calculate edge normal (perpendicular, pointing from first triangle toward next)
                    vec2 edgeNormal = vec2(-edgeDir.y, edgeDir.x) / edgeLen;  // Rotate 90 degrees counterclockwise
                    
                    // Determine which direction is "outward" - normal should point from triangle center toward next triangle center
                    vec2 toNextCenter = glm::normalize(nextTriangle.center - triangle.center);
                    float dot = glm::dot(edgeNormal, toNextCenter);
                    
                    // If normal points away from next triangle, flip it
                    if (dot < 0.0f) {
                        edgeNormal = -edgeNormal;
                    }
                    
                    // Push both endpoints outward along the normal
                    portalLeft += edgeNormal * padding;
                    portalRight += edgeNormal * padding;
                }
            }
            
            // Store portal endpoints
            portals.push_back({portalLeft, portalRight});
        }
    }
}

void Navmesh::generateNavmesh() {
    if (mesh.empty()) {
        return;
    }
    
    // Earcut the mesh
    earcut();
    
    if (triangles.empty()) {
        return;
    }
    
    // Build adjacency graph
    buildGraph();
}