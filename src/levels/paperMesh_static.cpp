#include "levels/levels.h"
#include "util/maths.h"

std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> PaperMesh::obstacleTemplates;


void addLines(std::vector<UVRegion>& obst, std::vector<vec2> points, Game* game) {
    if (points.size() % 2) throw std::runtime_error("addLines: points must be even");

    for (size_t i = 0; i < points.size(); i += 2) {
        std::pair<glm::vec3, glm::vec2> data = connectSquare(points[i], points[i + 1]);
        obst.push_back({ game->getMesh("quad"), data.first, data.second, true });
    }
}

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

    obstacleTemplates["notebook1_front"] = [game]() { 
        std::vector<UVRegion> obst;

        // how to make a single line
        std::pair<glm::vec3, glm::vec2> data = connectSquare(vec2{ 3.38, 2 }, vec2{ 8.88, 2 });
        obst.push_back({ game->getMesh("quad"), data.first, data.second, true });


        return obst; 
    };

    obstacleTemplates["notebook2_front"] = [game]() {
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2{ 3.38, 2 }, vec2{ 8.88, 2 },
        };
        
        addLines(obst, points, game);
        return obst; 
    };
}