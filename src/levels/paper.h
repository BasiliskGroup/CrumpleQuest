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
    struct PaperMesh : public DyMesh {
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

    struct Fold {
        DyMesh* underside;
        DyMesh* cover;
        std::set<int> holds;
        int side; 

        Fold(PaperMesh* paperMesh, const vec2& creasePos, const vec2& foldDir, const vec2& edgeIntersectPaper, int side=0);
        ~Fold();

        Fold(const Fold& other);
        Fold(Fold&& other) noexcept;
        Fold& operator=(const Fold& other);
        Fold& operator=(Fold&& other) noexcept;
 
        bool isCovered() { return holds.size() > 0; }
    };

public: // DEBUG
    
    // tracking folding
    std::vector<Fold> folds; // TODO, switch to ptrs
    int activeFold = NULL_FOLD;

    // side pairs
    std::pair<SingleSide*, SingleSide*> sides;
    std::pair<PaperMesh*, PaperMesh*> paperMeshes;
    short curSide;

    // TODO temporary
    Game* game = nullptr;
    std::vector<Node2D*> regionNodes;

    // tracking gameplay
    bool isOpen;

public:
    Paper();
    Paper(Mesh* mesh, const std::vector<vec2>& region);
    
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
    PaperMesh* getPaperMesh() { return curSide == 0 ? paperMeshes.first : paperMeshes.second; }

    void pushFold(Fold& newFold);
    void popFold(); // uses activeFold index
};

#endif