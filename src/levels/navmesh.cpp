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
    
    // std::cout << "[Navmesh::buildGraph] Building graph for " << triangles.size() << " triangles" << std::endl;

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
    
    // Count isolated triangles and edges without matches
    int isolatedTriangles = 0;
    int unmatchedEdges = edgeMap.size();
    for (uint i = 0; i < triangles.size(); i++) {
        if (triangles[i].adjacency.empty()) {
            isolatedTriangles++;
        }
    }
    
    // std::cout << "[Navmesh::buildGraph] Graph built. Isolated triangles: " << isolatedTriangles 
    //           << ", Unmatched edges: " << unmatchedEdges << std::endl;
    // for (uint i = 0; i < triangles.size(); i++) {
    //     if (triangles[i].adjacency.size() == 0) {
    //         std::cout << "[Navmesh::buildGraph] WARNING: Triangle " << i << " has no adjacencies!" << std::endl;
    //     }
    // }
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
            // std::cout << "[Navmesh::posToTriangle] Position (" << pos.x << ", " << pos.y 
            //           << ") found in triangle " << i << " (distance: " << dist << ")" << std::endl;
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
        // std::cout << "[Navmesh::posToTriangle] WARNING: Position (" << pos.x << ", " << pos.y 
        //           << ") not in any triangle (using tighter epsilon). Closest triangle " << closestIdx 
        //           << " at distance " << closestDist << std::endl;
        // std::cout << "[Navmesh::posToTriangle] Closest triangle center: (" 
        //           << triangles[closestIdx].center.x << ", " << triangles[closestIdx].center.y << ")" << std::endl;
        
        // If very close, use it anyway (tolerance for floating point errors)
        if (closestDist < 0.01f) {
            // std::cout << "[Navmesh::posToTriangle] Using closest triangle due to small distance" << std::endl;
            return closestIdx;
        }
    } else {
        // std::cout << "[Navmesh::posToTriangle] ERROR: Position (" << pos.x << ", " << pos.y 
        //           << ") - no triangles exist!" << std::endl;
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
    
    // std::cout << "[Navmesh::clear] Cleared all data" << std::endl;
}

void Navmesh::getPath(std::vector<vec2>& path, vec2 start, vec2 dest) {
    path.clear();
    
    // std::cout << "[Navmesh::getPath] Starting path from (" << start.x << ", " << start.y 
    //           << ") to (" << dest.x << ", " << dest.y << ")" << std::endl;
    // std::cout << "[Navmesh::getPath] Total triangles: " << triangles.size() << std::endl;
    
    // Reset all algorithm structures to ensure clean state
    resetAlgoStructs();
    
    // Run A* to get triangle path
    std::vector<uint> trianglePath;
    AStar(start, dest, trianglePath);
    
    if (trianglePath.empty()) {
        // std::cout << "[Navmesh::getPath] No path found (trianglePath empty)" << std::endl;
        return; // No path found
    }
    
    // std::cout << "[Navmesh::getPath] Triangle path found with " << trianglePath.size() 
    //           << " triangles" << std::endl;
    
    // Get portals from triangle path
    getPortals(trianglePath);
    
    // std::cout << "[Navmesh::getPath] Generated " << portals.size() << " portals" << std::endl;
    
    // Run funnel algorithm to smooth path
    funnel(start, dest, path);
    
    // std::cout << "[Navmesh::getPath] Final path has " << path.size() << " waypoints" << std::endl;
    // if (!path.empty()) {
    //     std::cout << "[Navmesh::getPath] First waypoint: (" << path[0].x << ", " << path[0].y << ")" << std::endl;
    //     if (path.size() > 1) {
    //         std::cout << "[Navmesh::getPath] Last waypoint: (" << path.back().x << ", " << path.back().y << ")" << std::endl;
    //     }
    // }
}

void Navmesh::AStar(const vec2& startPos, const vec2& destPos, std::vector<uint>& path) {
    // Convert positions to triangles
    int start = posToTriangle(startPos);
    int dest = posToTriangle(destPos);
    path.clear();

    // std::cout << "[Navmesh::AStar] Start triangle: " << start << ", Dest triangle: " << dest << std::endl;

    // Check if we found valid pathing locations
    if (start < 0 || dest < 0) {
        // std::cout << "[Navmesh::AStar] ERROR: Invalid triangle indices (start=" << start 
        //           << ", dest=" << dest << ")" << std::endl;
        return;
    }

    // Same triangle optimization
    if (start == dest) {
        // std::cout << "[Navmesh::AStar] Start and dest in same triangle" << std::endl;
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
            // std::cout << "[Navmesh::AStar] Found path! Reconstructing..." << std::endl;
            path.push_back(curIdx);
            uint pathLength = 1;
            while (cameFrom.find(curIdx) != cameFrom.end()) {
                curIdx = cameFrom[curIdx];
                path.push_back(curIdx);
                pathLength++;
            }
            std::reverse(path.begin(), path.end());
            // std::cout << "[Navmesh::AStar] Path reconstructed with " << pathLength << " triangles" << std::endl;
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
    // std::cout << "[Navmesh::AStar] ERROR: No path found! Open set exhausted. Closed set size: " 
    //           << closed.size() << ", Triangles with no adjacency: ";
    int isolatedCount = 0;
    for (uint i = 0; i < triangles.size(); i++) {
        if (triangles[i].adjacency.empty()) {
            isolatedCount++;
        }
    }
    // std::cout << isolatedCount << std::endl;
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
            
            // Store portal endpoints directly from edge (like Python reference: portals.append((a, b)))
            // Just use the edge vertices in order - funnel will handle orientation
            portals.push_back({edge.first, edge.second});
            
            // std::cout << "[Navmesh::getPortals] Portal " << i << " from triangle " << path[i] 
            //           << " (center: " << triangle.center.x << ", " << triangle.center.y << ")"
            //           << " to " << path[i+1] << " (center: " << nextTriangle.center.x << ", " << nextTriangle.center.y << ")"
            //           << ": edge (" << edge.first.x << ", " << edge.first.y 
            //           << ") -> (" << edge.second.x << ", " << edge.second.y << ")" << std::endl;
        } else {
            // std::cout << "[Navmesh::getPortals] ERROR: Triangle " << path[i] 
            //           << " has no adjacency to triangle " << path[i + 1] << std::endl;
        }
    }
    
    // std::cout << "[Navmesh::getPortals] Generated " << portals.size() << " portals from " 
    //           << path.size() << " triangles" << std::endl;
}

void Navmesh::funnel(const vec2& start, const vec2& dest, std::vector<vec2>& path) {
    path.clear();
    path.push_back(start);

    // If we are in the same triangle, take the direct path
    if (portals.empty()) {
        path.push_back(dest);
        return;
    }

    // Add dummy portals at start and end (like Python implementation)
    // This ensures the funnel properly processes start and end constraints
    std::vector<Edge> extendedPortals;
    extendedPortals.reserve(portals.size() + 2);
    extendedPortals.push_back({start, start});  // Dummy start portal
    extendedPortals.insert(extendedPortals.end(), portals.begin(), portals.end());
    extendedPortals.push_back({dest, dest});    // Dummy end portal

    // Initialize funnel - portals should be (left, right) when viewed from start moving toward dest
    uint apexIndex = 0;
    uint leftIndex = 0;
    uint rightIndex = 0;

    vec2 apex = start;
    vec2 left = extendedPortals[0].first;   // Start portal (left = right = start)
    vec2 right = extendedPortals[0].second; // Start portal
    
    // std::cout << "[Navmesh::funnel] Starting funnel with " << portals.size() 
    //           << " real portals (extended to " << extendedPortals.size() << ")" << std::endl;
    // std::cout << "[Navmesh::funnel] Initial left: (" << left.x << ", " << left.y 
    //           << "), right: (" << right.x << ", " << right.y << ")" << std::endl;

    // Process all portals (starting from index 1, which is the first real portal)
    for (uint i = 1; i < extendedPortals.size(); i++) {
        vec2 newLeft = extendedPortals[i].first;
        vec2 newRight = extendedPortals[i].second;

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
                i = apexIndex;
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
                i = apexIndex;
                continue;
            }
        }
    }

    // Always append destination (like Python implementation)
    path.push_back(dest);
    
    // std::cout << "[Navmesh::funnel] Funnel complete. Final path points: " << path.size() << std::endl;
    // for (size_t j = 0; j < path.size(); j++) {
    //     std::cout << "  [" << j << "] (" << path[j].x << ", " << path[j].y << ")" << std::endl;
    // }
}

void Navmesh::generateNavmesh() {
    if (mesh.empty()) {
        // std::cout << "[Navmesh::generateNavmesh] WARNING: Mesh is empty!" << std::endl;
        return;
    }
    
    // std::cout << "[Navmesh::generateNavmesh] Generating navmesh with " << mesh.size() 
    //           << " vertices and " << rings.size() << " rings" << std::endl;
    
    // Earcut the mesh
    earcut();
    
    // std::cout << "[Navmesh::generateNavmesh] Earcut produced " << triangles.size() << " triangles" << std::endl;
    
    if (triangles.empty()) {
        // std::cout << "[Navmesh::generateNavmesh] ERROR: No triangles generated!" << std::endl;
        return;
    }
    
    // Build adjacency graph
    buildGraph();
    
    // std::cout << "[Navmesh::generateNavmesh] Navmesh generation complete" << std::endl;
}