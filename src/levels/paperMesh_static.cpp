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
            vec2(1.84,5.5), vec2(1.94,2.1),
            vec2(10.16,6.9), vec2(1.94,2.1),
            vec2(10.16,6.9), vec2(10.16,3.55),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook3_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.13,8.9), vec2(4.2,6),
            vec2(0.13,2.58), vec2(4.16,2.58),
            vec2(11.84,6.44), vec2(8,6.47),
            vec2(11.8,0.12), vec2(8.03,3.26),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(9.17,6.37), vec2(2.86,6.4),
            vec2(9.1,2.55), vec2(3,2.57),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook4_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.22,0.13), vec2(3.45,2.53),
            vec2(0.2,8.8), vec2(3.5,6.4),
            vec2(11.87,8.9), vec2(8.44,6.4),
            vec2(11.9,0.2), vec2(8.4,2.56),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(11.77,6.1), vec2(0.2,6.1),
            vec2(11.84,2.93), vec2(0.2,2.78),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["notebook5_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(4.1,0.18), vec2(4,8.88),
            vec2(7.85,0.2), vec2(7.98,8.95),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid1_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(3.6,6.43), vec2(8.4,6.33),
            vec2(3.6,6.43), vec2(3.77,2.67),
            vec2(8.4,2.7), vec2(3.77,2.67),
            vec2(8.4,2.7), vec2(8.4,6.3),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid1_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(3.92,5.92), vec2(0.14,5.9),
            vec2(3.92,5.92), vec2(3.96,8.87),
            vec2(8.1,5.8), vec2(8.05,8.8),
            vec2(8.1,5.8), vec2(11.86,5.73),
            vec2(8.1,3.1), vec2(11.75,3.07),
            vec2(8.1,3.1), vec2(8.04,0.06),
            vec2(4,2.96), vec2(4,0.13),
            vec2(4,2.96), vec2(0.2,3.03),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid2_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(5.8,6.1), vec2(0.2,0.17),
            vec2(3.03,5.73), vec2(0.16,8.9),
            vec2(11.88,8.77), vec2(6.1,2.68),
            vec2(9.2,3.1), vec2(11.77,0.17),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid2_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.1,3.07), vec2(1.03,2.3),
            vec2(3.15,0.13), vec2(2.4,0.87),
            vec2(9.56,1), vec2(8.7,0.1),
            vec2(10.78,2.3), vec2(11.84,3.37),
            vec2(10.87,6.7), vec2(11.94,5.5),
            vec2(9.75,7.8), vec2(8.77,8.86),
            vec2(3.26,8.86), vec2(2.46,8.1),
            vec2(0.1,5.75), vec2(1.07,6.67),
            vec2(8.25,6), vec2(3.73,5.94),
            vec2(8.13,3), vec2(3.67,2.93),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid3_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(2.86,2), vec2(0.14,0.13),
            vec2(0.1,7.07), vec2(9.13,0.1),
            vec2(3.6,8.93), vec2(11.83,2.62),
            vec2(9.4,7.1), vec2(11.9,8.87),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid3_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(2.86,2), vec2(0.14,0.13),
            vec2(0.1,7.07), vec2(9.13,0.1),
            vec2(3.6,8.93), vec2(11.83,2.62),
            vec2(9.4,7.1), vec2(11.9,8.87),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid4_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.13,0.13), vec2(4.3,4.46),
            vec2(4.26,7.03), vec2(4.3,4.46),
            vec2(4.26,7.03), vec2(1.86,6.98),
            vec2(1.9,5.1), vec2(1.86,6.98),
            vec2(10.27,3.63), vec2(10.23,1.87),
            vec2(7.82,1.8), vec2(10.23,1.87),
            vec2(7.82,1.8), vec2(7.67,4.2),
            vec2(11.88,8.75), vec2(7.67,4.2),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid4_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.84,7), vec2(1.8,5.3),
            vec2(1.84,7), vec2(4.35,7.1),
            vec2(10.1,3.92), vec2(4.35,7.1),
            vec2(10.1,3.92), vec2(10.2,1.93),
            vec2(8,1.86), vec2(10.2,1.93),
            vec2(8,1.86), vec2(1.76,5.32),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid5_front"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(1.83,8.9), vec2(1.83,0.13),
            vec2(4.17,8.96), vec2(4.18,0.16),
            vec2(7.84,8.9), vec2(7.85,0.16),
            vec2(10.1,8.83), vec2(10.2,0.14),
        };
        
        addLines(obst, points, game);
        return obst; 
    };

    obstacleTemplates["grid5_back"] = [game]() { 
        std::vector<UVRegion> obst;

        std::vector<vec2> points = { 
            vec2(0.03,8.93), vec2(4.35,4.55),
            vec2(0.1,0.08), vec2(4.35,4.55),
            vec2(7.6,4.63), vec2(11.9,8.85),
            vec2(7.6,4.63), vec2(11.7,0.2),
            vec2(3.36,0.1), vec2(4.88,1.67),
            vec2(8.63,0.13), vec2(7.1,1.76),
            vec2(4.78,7.47), vec2(3.37,8.9),
            vec2(7.2,7.44), vec2(8.7,8.9),
        };
        
        addLines(obst, points, game);
        return obst; 
    };
}