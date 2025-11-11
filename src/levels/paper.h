#ifndef PAPER_H
#define PAPER_H

#include "util/includes.h"
#include "levels/singleSide.h"
#include "levels/triangle.h"
#include "levels/edger.h"
#include "levels/dymesh.h"

class Game;

class Paper {
public:
    static std::unordered_map<std::string, Paper> templates;
    static std::unordered_map<RoomTypes, std::vector<std::string>> papers;

private:
    struct Fold : public DyMesh {
        std::set<Fold> holds;
        vec2 crease;
        int layer;
        int side; 

        Fold(const std::vector<vec2>& verts, vec2 crease, int layer, int side=0);
        bool contains(const vec2& pos);
        
        // Fold needs operator< for std::set, if not already defined
        bool operator<(const Fold& other) const {
            if (layer != other.layer) return layer < other.layer;
            if (side != other.side) return side < other.side;
            return crease.x < other.crease.x || (crease.x == other.crease.x && crease.y < other.crease.y);
        }
    };

    struct PaperMesh : public DyMesh {
        Mesh* mesh;

        PaperMesh(const std::vector<vec2> verts, const std::vector<Vert>& data);
        ~PaperMesh();
        
        // Rule of 5 for PaperMesh
        PaperMesh(const PaperMesh& other);
        PaperMesh(PaperMesh&& other) noexcept;
        PaperMesh& operator=(const PaperMesh& other);
        PaperMesh& operator=(PaperMesh&& other) noexcept;
    };
    
    // tracking folding
    std::vector<Fold> folds;
    int activeFold = -1;

    // side pairs
    std::pair<SingleSide*, SingleSide*> sides;
    std::pair<PaperMesh*, PaperMesh*> paperMeshes;
    short curSide;

    // TODO temporary
    Game* game = nullptr;

    // tracking gameplay
    bool isOpen;

public:
    Paper();
    Paper(Mesh* mesh, const std::vector<vec2>& edgeVerts);
    Paper(SingleSide* sideA, SingleSide* sideB, short startSide=0, bool isOpen=false);
    
    // Rule of 5
    Paper(const Paper& other);
    Paper(Paper&& other) noexcept;
    ~Paper();
    Paper& operator=(const Paper& other);
    Paper& operator=(Paper&& other) noexcept;

    // getters
    Mesh* getMesh();
    SingleSide* getSingleSide() { return curSide ? sides.second : sides.first; }

    void flip();
    void open();
    void fold(const vec2& start, const vec2& end);

    void activateFold(const vec2& start);
    void deactivateFold();

    // TODO temporary
    void setGame(Game* game) { this->game = game; }

    static void generateTemplates(Game* game);
    static const Paper& getRandomTemplate(RoomTypes type);
    static void flattenVertices(const std::vector<Vert>& vertices, std::vector<float>& data);

private:
    void clear();
    void initFolds(const std::vector<vec2>& edgeVerts);
};

#endif