#ifndef PAPER_H
#define PAPER_H

#include "util/includes.h"
#include "levels/singleSide.h"
#include "levels/triangle.h"

class Paper {
public:
    static std::unordered_map<std::string, Paper> templates;
    static std::unordered_map<RoomTypes, std::vector<std::string>> papers;

private:
    struct Fold {
        std::vector<Tri> triangles;
        std::set<Fold> holds;
        vec2 crease;
        int layer;

        Fold(const std::vector<vec2>& vertices, vec2 crease, int layer);
        bool contains(const vec2& pos);
    };

    struct PaperMesh {
        std::vector<Vert> verts;
        Mesh* mesh;

        PaperMesh();
        PaperMesh(const std::vector<Vert>& verts);
        ~PaperMesh();
    };
    
    // tracking folding
    std::vector<Fold> folds;
    Fold* activeFold = nullptr;

    // side pairs
    std::pair<SingleSide*, SingleSide*> sides;
    std::pair<PaperMesh*, PaperMesh*> paperMeshes;
    short curSide;

    // tracking gameplay
    bool isOpen;

public:
    Paper();
    Paper(Mesh* mesh);
    Paper(SingleSide* sideA, SingleSide* sideB, short startSide=0, bool isOpen=false);
    Paper(const Paper& other) noexcept;
    Paper(Paper&& other) noexcept;
    ~Paper();

    Paper& operator=(const Paper& other) noexcept;
    Paper& operator=(Paper&& other) noexcept;

    // getters
    Mesh* getMesh();
    SingleSide* getSingleSide() { return curSide ? sides.second : sides.first; }

    void flip();
    void open();

    static void generateTemplates(Game* game);
    static const Paper& getRandomTemplate(RoomTypes type);
    static void flattenVertices(const std::vector<Vert>& vertices, std::vector<float>& data);

private:
    void clear();
    void initFolds();
};

#endif