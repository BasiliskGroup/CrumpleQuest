#include "levels/levels.h"

PaperMesh::PaperMesh(const std::vector<vec2> verts, Mesh* mesh) 
    : DyMesh(verts, mesh), mesh(nullptr), navmesh(nullptr)
{
    // Create mesh to render
    std::vector<float> data; 
    toData(data);
    this->mesh = new Mesh(data);

    // Create navmesh for enemy AI
    navmesh = new Navmesh(verts);
    regenerateNavmesh();

    startingRegion = region;
}

PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
    delete navmesh;
    navmesh = nullptr;
}

PaperMesh::PaperMesh(const PaperMesh& other) 
    : DyMesh(other.region, other.regions), mesh(nullptr), navmesh(nullptr), startingRegion(other.startingRegion)
{
    std::vector<float> data;
    toData(data);
    mesh = new Mesh(data);

    navmesh = new Navmesh(region);
    regenerateNavmesh();
}

PaperMesh::PaperMesh(PaperMesh&& other) noexcept 
    : DyMesh(std::move(other.region), std::move(other.regions)), 
      mesh(other.mesh), 
      navmesh(other.navmesh),
      startingRegion(other.startingRegion)
{
    other.mesh = nullptr;
    other.navmesh = nullptr;
}

PaperMesh& PaperMesh::operator=(const PaperMesh& other) {
    if (this == &other) return *this;
    
    // Copy-and-swap idiom for exception safety
    PaperMesh temp(other);
    
    delete mesh;
    delete navmesh;
    
    mesh = temp.mesh;
    navmesh = temp.navmesh;
    region = std::move(temp.region);
    regions = std::move(temp.regions);
    startingRegion = std::move(temp.startingRegion);
    
    temp.mesh = nullptr;
    temp.navmesh = nullptr;
    
    return *this;
}

PaperMesh& PaperMesh::operator=(PaperMesh&& other) noexcept {
    if (this == &other) return *this;
    
    delete mesh;
    delete navmesh;
    
    region = std::move(other.region);
    regions = std::move(other.regions);
    startingRegion = std::move(other.startingRegion);
    mesh = other.mesh;
    navmesh = other.navmesh;
    
    other.mesh = nullptr;
    other.navmesh = nullptr;
    
    return *this;
}

void PaperMesh::regenerateMesh() {
    Mesh* oldPaperMesh = mesh;
    std::vector<float> newMeshData;
    toData(newMeshData);
    mesh = new Mesh(newMeshData);
    delete oldPaperMesh;
}

void PaperMesh::regenerateNavmesh() {
    if (!navmesh) return;
    
    navmesh->clear();
    navmesh->addMesh(region);
    
    // Add only UV regions marked as obstacles
    for (const auto& uvRegion : regions) {
        if (uvRegion.isObstacle) {
            navmesh->addObstacle(uvRegion.positions);
        }
    }
    
    navmesh->generateNavmesh();
}

bool PaperMesh::hasLineOfSight(const vec2& start, const vec2& end) const {
    // Check if the line segment intersects any obstacle edge
    for (const auto& uvRegion : regions) {
        // Only check regions marked as obstacles
        if (!uvRegion.isObstacle) {
            continue;
        }
        
        // Skip regions with insufficient vertices
        if (uvRegion.positions.size() < 3) {
            continue;
        }
        
        // Check if line segment intersects this obstacle polygon
        if (lineSegmentIntersectsPolygon(start, end, uvRegion.positions)) {
            return false; // Line of sight blocked
        }
    }
    
    return true; // No obstacles blocking the line
}

std::pair<vec2, vec2> PaperMesh::getAABB() {
    vec2 bl = vec2{ std::numeric_limits<float>::infinity() };
    vec2 tr = vec2{ -std::numeric_limits<float>::infinity() };

    for (const auto& r : startingRegion) {
        if (r.x < bl.x) bl.x = r.x;
        if (r.y < bl.y) bl.y = r.y;
        if (r.x > tr.x) tr.x = r.x;
        if (r.y > tr.y) tr.y = r.y;
    }

    return { bl, tr };
}