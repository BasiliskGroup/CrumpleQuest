#include "levels/levels.h"

std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> PaperMesh::obstacleTemplates;

void PaperMesh::generateTemplates(Game* game) {

    obstacleTemplates["empty"] = [game]() { 
        std::vector<UVRegion> obst;
        return obst; 
    };

    obstacleTemplates["squareMiddle"] = [game]() { 
        std::vector<UVRegion> obst;
        obst.push_back({ game->getMesh("quad"), { 0, 0, 0 }, { 3, 3 }, true });
        return obst; 
    };
}