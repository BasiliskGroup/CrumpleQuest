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
    
    std::cout << "[PaperMesh::regenerateNavmesh] Total regions: " << regions.size() << std::endl;
    
    // Add only UV regions marked as obstacles
    int obstacleCount = 0;
    for (size_t i = 0; i < regions.size(); i++) {
        const auto& uvRegion = regions[i];
        std::cout << "[PaperMesh::regenerateNavmesh] Region " << i 
                  << ": isObstacle=" << uvRegion.isObstacle 
                  << ", positions=" << uvRegion.positions.size() << std::endl;
        if (uvRegion.isObstacle) {
            obstacleCount++;
            std::cout << "[PaperMesh::regenerateNavmesh] Adding obstacle " << obstacleCount 
                      << " with " << uvRegion.positions.size() << " vertices" << std::endl;
            navmesh->addObstacle(uvRegion.positions);
        }
    }
    
    std::cout << "[PaperMesh::regenerateNavmesh] Found " << obstacleCount << " obstacles" << std::endl;
    navmesh->generateNavmesh();
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