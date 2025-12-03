#ifndef PAPER_H
#define PAPER_H

#include "util/includes.h"
#include "levels/singleSide.h"
#include "levels/triangle.h"
#include "levels/edger.h"
#include "levels/dymesh.h"
#include "levels/paperMesh.h"

class Game;

class Paper {
public:
    static std::unordered_map<std::string, std::function<Paper*()>> templates;
    static std::unordered_map<RoomTypes, std::vector<std::string>> papers;
    static void generateTemplates(Game* game);
    static Paper* getRandomTemplate(RoomTypes type);
    static void flattenVertices(const std::vector<Vert>& vertices, std::vector<float>& data); // TODO move to generic helper

private:
    using PaperMeshPair = std::pair<PaperMesh*, PaperMesh*>;

    struct Fold {
        DyMesh* underside;
        DyMesh* backside;
        DyMesh* cover;
        std::set<int> holds;
        vec2 start;
        int side; 
        Vec2Pair crease;
        vec2 creasePos;
        vec2 creaseDir;

        // cleaning
        std::vector<vec2> cleanVerts;
        std::vector<vec2> cutVerts;  // Original corner region being folded
        std::vector<vec2> originalFoldedVerts;  // Original vertices from indexBounds that were folded

        Fold(const vec2& start, int side=0);
        ~Fold();

        bool initialize(PaperMeshPair meshes, const vec2& creasePos, const vec2& foldDir, const vec2& edgeIntersectPaper);

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
    PaperMeshPair paperMeshes;
    short curSide;

    // game back ref
    Game* game = nullptr;
    std::vector<Node2D*> regionNodes;

    // tracking gameplay
    bool isOpen;

public:
    Paper();
    Paper(Mesh* mesh0, Mesh* mesh1, const std::vector<vec2>& region, std::pair<std::string, std::string> sideNames, std::pair<std::string, std::string> obstacleNames);
    
    // Rule of 5
    Paper(const Paper& other);
    Paper(Paper&& other) noexcept;
    ~Paper();
    Paper& operator=(const Paper& other);
    Paper& operator=(Paper&& other) noexcept;

    // getters
    Mesh* getMesh();
    SingleSide* getSingleSide() { return curSide == 0 ? sides.first : sides.second; }

    void flip();
    void open();
    void fold(const vec2& start, const vec2& end);
    bool unfold(const vec2& pos);

    void activateFold(const vec2& start);
    void deactivateFold();
    void regenerateWalls();
    void regenerateWalls(int side);

    void setGame(Game* game) { this->game = game; }
    void previewFold(const vec2& start, const vec2& end);  // Preview fold cover without applying
    void toData(std::vector<float>& out);

    // enemies
    void updatePathing(vec2 playerPos);

    // DEBUG
    void dotData();

private:
    // Shared fold validation and geometry calculation
    struct FoldGeometry {
        vec2 foldDir;
        vec2 creasePos;
        vec2 edgeIntersectPaper;
        vec2 nearEdgePointPaper;
        bool isValid;
    };
    FoldGeometry validateFoldGeometry(const vec2& start, const vec2& end);

private:
    void clear();
    PaperMesh* getPaperMesh() { return curSide == 0 ? paperMeshes.first : paperMeshes.second; }
    PaperMesh* getBackPaperMesh() { return curSide == 0 ? paperMeshes.second : paperMeshes.first; }

    void pushFold(Fold& newFold);
    bool popFold(); // uses activeFold index
};

#endif