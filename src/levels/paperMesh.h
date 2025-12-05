#ifndef PAPER_MESH_H
#define PAPER_MESH_H

#include "util/includes.h"
#include "levels/edger.h"
#include "levels/dymesh.h"
#include "levels/navmesh.h"

class Game;

struct PaperMesh : public DyMesh {
    static std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> obstacleTemplates;
    static void generateTemplates(Game* game);

    Mesh* mesh;
    std::vector<vec2> startingRegion;
    Navmesh* navmesh;

    PaperMesh(const std::vector<vec2> verts, Mesh* mesh);
    ~PaperMesh();
    
    // Rule of 5 for PaperMesh
    PaperMesh(const PaperMesh& other);
    PaperMesh(PaperMesh&& other) noexcept;
    PaperMesh& operator=(const PaperMesh& other);
    PaperMesh& operator=(PaperMesh&& other) noexcept;

    void regenerateMesh();
    void regenerateNavmesh();

    void getPath(std::vector<vec2>& path, vec2 start, vec2 dest, float padding = 0.0f) {
        if (navmesh) navmesh->getPath(path, start, dest, padding);
    }

    bool hasLineOfSight(const vec2& start, const vec2& end) const;

    std::pair<vec2, vec2> getAABB() const;

    // Update obstacle UVs to match surrounding paperMesh regions
    void updateObstacleUVs(std::vector<UVRegion>& obstacles);

    // Select a random position that is not in any UVregion marked as isObstacle
    vec2 getRandomNonObstaclePosition(int maxAttempts = 1000) const;
};

#endif