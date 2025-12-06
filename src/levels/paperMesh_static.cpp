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
            vec2(0,4.33), vec2(1.82,6.87),
            vec2(3.67,3.4), vec2(1.82,6.87),
            vec2(3.67,3.4), vec2(7.9,2.5),
            vec2(4.8,6.62), vec2(6.68,2.78),
            vec2(4.8,6.62), vec2(9.5,5.54),
            vec2(6.65,0.1), vec2(9.5,5.54),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook2_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(2.64,3.73), vec2(1.14,8.86),
            vec2(2.64,3.73), vec2(6.56,6.62),
            vec2(3.72,6.95), vec2(6.56,6.62),
            vec2(3.72,6.95), vec2(7.2,1.48),
            vec2(6.27,0.1), vec2(9.8,4.9),
            vec2(11.93,0.7), vec2(9.8,4.9),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook3_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.2,4.34), vec2(4.73,8.92),
            vec2(0.2,4.34), vec2(4.35,0.07),
            vec2(7.62,0.1), vec2(11.83,4.23),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook3_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.2,8.84), vec2(11.9,0.4),
            vec2(9.75,7.02), vec2(0.2,0.24),
            vec2(9.75,7.02), vec2(2.7,7.04),
            vec2(9.75,7.02), vec2(9.84,1.76),
            vec2(4.23,3.07), vec2(4.3,0.1),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.03,4.6), vec2(11.86,4.63),
            vec2(5.45,0.06), vec2(5.4,2.1),
            vec2(5.4,8.87), vec2(5.4,6.54),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.1,6.67), vec2(4.27,6.7),
            vec2(0.07,4.5), vec2(4.6,4.57),
            vec2(0.13,2.3), vec2(4.5,2.34),
            vec2(7.6,0.03), vec2(7.57,8.88),
            vec2(11.87,0.3), vec2(7.57,4.43),
            vec2(11.97,8.77), vec2(7.57,4.43),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(2.53,1.46), vec2(0.06,0.1),
            vec2(2.53,1.46), vec2(0.1,2.8),
            vec2(2.53,4.43), vec2(0.1,2.8),
            vec2(2.53,4.43), vec2(0.17,5.35),
            vec2(2.53,6.77), vec2(0.17,5.35),
            vec2(2.53,6.77), vec2(0.14,8.63),
            vec2(9.2,7.44), vec2(11.8,8.83),
            vec2(9.2,7.44), vec2(11.87,5.83),
            vec2(9.3,4.26), vec2(11.87,5.83),
            vec2(9.3,4.26), vec2(11.7,2.8),
            vec2(9.44,1.7), vec2(11.7,2.8),
            vec2(9.44,1.7), vec2(11.73,0.1),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.27,1.9), vec2(0,0.1),
            vec2(1.27,1.9), vec2(2.45,0.1),
            vec2(3.96,2.07), vec2(2.45,0.1),
            vec2(3.96,2.07), vec2(5.37,0.1),
            vec2(6.86,2.23), vec2(5.37,0.1),
            vec2(6.86,2.23), vec2(8.38,0.13),
            vec2(9.73,2.1), vec2(8.38,0.13),
            vec2(9.73,2.1), vec2(11.73,0.06),
            vec2(0.1,8.75), vec2(1.4,6.43),
            vec2(2.48,8.64), vec2(1.4,6.43),
            vec2(2.48,8.64), vec2(3.95,6.5),
            vec2(5.2,8.8), vec2(3.95,6.5),
            vec2(5.2,8.8), vec2(6.75,6.6),
            vec2(8.43,8.77), vec2(6.75,6.6),
            vec2(8.43,8.77), vec2(9.94,6.6),
            vec2(11.65,8.77), vec2(9.94,6.6),
        };
        
        addLines(obst, points, game);
        return obst; 
    };
}