#include "levels/levels.h"
#include "util/maths.h"

std::unordered_map<std::string, std::function<std::vector<UVRegion>()>> PaperMesh::obstacleTemplates;


void addLines(std::vector<UVRegion>& obst, std::vector<vec2> points, Game* game) {
    if (points.size() % 2) throw std::runtime_error("addLines: points must be even");

    vec2 offset = vec2(6.0f, 4.5f); 

    for (size_t i = 0; i < points.size(); i += 2) {
        std::pair<glm::vec3, glm::vec2> data = connectSquare(points[i] - offset, points[i + 1] - offset, 0.1f);
        obst.push_back({ game->getMesh("quad"), data.first, data.second, true });
    }
}

void PaperMesh::generateTemplates(Game* game) {

    obstacleTemplates["tutorial_front"] = [game]() { 
        std::vector<UVRegion> obst;
        return obst; 
    };

    obstacleTemplates["tutorial_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(7.67,0.1), vec2(7.57,8.85),
        };

        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["empty"] = [game]() { 
        std::vector<UVRegion> obst;
        return obst; 
    };

    obstacleTemplates["notebook1_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(3.41,6.97), vec2(8.83,6.99),
            vec2(3.41,6.97), vec2(3.47,1.84),
            vec2(8.9,1.96), vec2(3.47,1.84),
            vec2(8.9,1.96), vec2(8.84,7),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook1_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(3.1,1.9), vec2(3,7),
            vec2(8.6,6.94), vec2(3,7),
            vec2(8.6,6.94), vec2(8.54,1.84),
            vec2(3.15,1.95), vec2(8.54,1.84),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook2_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.83,5.48), vec2(1.8,3.6),
            vec2(4.25,7.16), vec2(7.8,7.15),
            vec2(10.16,3.6), vec2(10.14,5.44),
            vec2(7.6,1.93), vec2(4.27,1.9),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook2_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.8,5.2), vec2(1.8,7.13),
            vec2(3.43,7.18), vec2(1.8,7.13),
            vec2(10.1,7.07), vec2(8.54,7.1),
            vec2(10.1,7.07), vec2(10.2,5.46),
            vec2(10.2,2.06), vec2(10.2,3.46),
            vec2(10.2,2.06), vec2(8.76,1.97),
            vec2(3.43,1.9), vec2(1.8,1.97),
            vec2(1.8,3.7), vec2(1.8,1.97),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook2_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.83,5.48), vec2(1.8,3.6),
            vec2(4.25,7.16), vec2(7.8,7.15),
            vec2(10.16,3.6), vec2(10.14,5.44),
            vec2(7.6,1.93), vec2(4.27,1.9),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook3_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2\left(1.84,5.5\right), vec2\left(1.94,2.1\right),
            vec2\left(10.16,6.9\right), vec2\left(1.94,2.1\right),
            vec2\left(10.16,6.9\right), vec2\left(10.16,3.55\right),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook3_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2\left(0.13,8.9\right), vec2\left(4.2,6\right),
            vec2\left(0.13,2.58\right), vec2\left(4.16,2.58\right),
            vec2\left(11.84,6.44\right), vec2\left(8,6.47\right),
            vec2\left(11.8,0.12\right), vec2\left(8.03,3.26\right),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2\left(9.17,6.37\right), vec2\left(2.86,6.4\right),
            vec2\left(9.1,2.55\right), vec2\left(3,2.57\right),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2\left(0.22,0.13\right), vec2\left(3.45,2.53\right),
            vec2\left(0.2,8.8\right), vec2\left(3.5,6.4\right),
            vec2\left(11.87,8.9\right), vec2\left(8.44,6.4\right),
            vec2\left(11.9,0.2\right), vec2\left(8.4,2.56\right),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
            vec2, vec2,
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    // obstacleTemplates["notebook1_front"] = [game]() { 
    //     std::vector<UVRegion> obst;

    //     std::vector<vec2> points = { 
    //         vec2(3.4,6.97), vec2(8.78,7.02),
    //         vec2(3.4,6.97), vec2(3.46,1.76),
    //         vec2(8.87,1.85), vec2(3.46,1.76),
    //         vec2(8.87,1.85), vec2(8.8,7),
    //     };
        
    //     addLines(obst, points, game);
    //     return obst; 
    // };

    // obstacleTemplates["notebook1_back"] = [game]() { 
    //     std::vector<UVRegion> obst;

    //     std::vector<vec2> points = { 
    //         vec2(3.67,5.36), vec2(0.16,5.36),
    //         vec2(3.67,5.36), vec2(3.57,8.96),
    //         vec2(0.00,3.40), vec2(3.52,3.46),
    //         vec2(3.52,3.46), vec2(3.63,0.04),
    //         vec2(8.37,5.20), vec2(8.3,8.92),
    //         vec2(8.37,5.20), vec2(11.84,5.13),
    //         vec2(8.43,3.60), vec2(11.88,3.6),
    //         vec2(8.43,3.60), vec2(8.36,0.01),
    //     };
        
    //     addLines(obst, points, game);
    //     return obst; 
    // };
}