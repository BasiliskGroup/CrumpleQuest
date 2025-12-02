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
}

PaperMesh::~PaperMesh() {
    delete mesh; 
    mesh = nullptr;
    delete navmesh;
    navmesh = nullptr;
}

PaperMesh::PaperMesh(const PaperMesh& other) 
    : DyMesh(other.region, other.regions), mesh(nullptr), navmesh(nullptr)
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
      navmesh(other.navmesh)
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