#ifndef PAPER_MESH_H
#define PAPER_MESH_H

#include "util/includes.h"
#include "levels/edger.h"
#include "levels/dymesh.h"

class Game;

struct PaperMesh : public DyMesh {
    static std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> obstacleTemplates;
    static void generateTemplates(Game* game);

    Mesh* mesh;

    PaperMesh(const std::vector<vec2> verts, Mesh* mesh);
    ~PaperMesh();
    
    // Rule of 5 for PaperMesh
    PaperMesh(const PaperMesh& other);
    PaperMesh(PaperMesh&& other) noexcept;
    PaperMesh& operator=(const PaperMesh& other);
    PaperMesh& operator=(PaperMesh&& other) noexcept;

    void regenerateMesh();
};

#endif