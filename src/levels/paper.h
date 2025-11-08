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

    struct PaperNode {
        std::vector<vec2> verts;
        Mesh* mesh;
        Node2D* node;

        PaperNode(const std::vector<vec2>& verts);
    };
    
    // tracking folding
    std::vector<Fold> folds;
    std::vector<vec2> meshVertices = {{10, 10}, {-10, 10}, {-10, -10}, {10, -10}};

    // tracking gameplay
    std::pair<SingleSide*, SingleSide*> sides;
    SingleSide* curSide;
    bool isOpen;

public:
    Paper();
    Paper(SingleSide* sideA, SingleSide* sideB, int startSide=0, bool isOpen=false);
    Paper(const Paper& other) noexcept;
    Paper(Paper&& other) noexcept;
    ~Paper();

    Paper& operator=(const Paper& other) noexcept;
    Paper& operator=(Paper&& other) noexcept;

    void flip();
    void open();

    static void generateTemplates(Game* game);
    static const Paper& getRandomTemplate(RoomTypes type);

private:
    void clear();
    void initFolds();
};

#endif