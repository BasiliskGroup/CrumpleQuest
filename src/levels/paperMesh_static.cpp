#include "levels/levels.h"
#include "util/maths.h"

std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> PaperMesh::obstacleTemplates;

void PaperMesh::generateTemplates(Game* game) {

    obstacleTemplates["empty"] = [game]() { 
        std::vector<UVRegion> obst;
        return obst; 
    };

    obstacleTemplates["squareMiddle"] = [game]() { 
        std::vector<UVRegion> obst;
        obst.push_back({ game->getMesh("quad"), { 0, 0, 0 }, { 12, 9 }, true });
        return obst; 
    };

    obstacleTemplates["notebook1_level1"] = [game]() { 
        std::vector<UVRegion> obst;

        // how to make a single line
        std::pair<glm::vec3, glm::vec2> data = connectSquare(vec2{ 0, 0 }, vec2{ 12, 9 });
        obst.push_back({ game->getMesh("quad"), data.first, data.second, true });

        
        return obst; 
    };
}