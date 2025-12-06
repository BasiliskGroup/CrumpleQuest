#include "levels/levels.h"
#include "util/maths.h"

std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> PaperMesh::obstacleTemplates;


void addLines(std::vector<UVRegion>& obst, std::vector<vec2> points, Game* game) {
    if (points.size() % 2) throw std::runtime_error("addLines: points must be even");

    vec2 offset = vec2(6.0f, 4.5f); 

    for (size_t i = 0; i < points.size(); i += 2) {
        std::pair<glm::vec3, glm::vec2> data = connectSquare(points[i] - offset, points[i + 1] - offset, 0.01f);
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

        std::vector<vec2> points = { 
            vec2(3.4,6.97), vec2(8.78,7.02),
            vec2(3.4,6.97), vec2(3.46,1.76),
            vec2(8.87,1.85), vec2(3.46,1.76),
            vec2(8.87,1.85), vec2(8.8,7),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook1_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(3.67,5.36), vec2(0.16,5.36),
            vec2(3.67,5.36), vec2(3.57,8.96),
            vec2(0.00,3.40), vec2(3.52,3.46),
            vec2(3.52,3.46), vec2(3.63,0.04),
            vec2(8.37,5.20), vec2(8.3,8.92),
            vec2(8.37,5.20), vec2(11.84,5.13),
            vec2(8.43,3.60), vec2(11.88,3.6),
            vec2(8.43,3.60), vec2(8.36,0.01),
        };
        
        addLines(obst, points, game);
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