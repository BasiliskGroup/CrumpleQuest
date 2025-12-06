#include "levels/levels.h"
#include "util/random.h"

std::unordered_map<std::string, std::function<Paper*(float difficulty)>> Paper::templates;
std::unordered_map<RoomTypes, std::vector<std::string>> Paper::papers;

void Paper::generateTemplates(Game* game) {
    // ---------------------
    // create templates
    // ---------------------

    templates["tutorial"] = [game](float difficulty) {
        return new Paper(game, { "tutorial_front", "tutorial_back" }, { "tutorial_front", "tutorial_back" }, difficulty);
    };

    templates["notebook_boss"] = [game](float difficulty) {
        return new Paper(game, { "notebook_boss_front", "notebook_boss_back" }, { "empty", "empty" }, difficulty);
    };
    templates["grid_boss"] = [game](float difficulty) {
        return new Paper(game, { "grid_boss_front", "grid_boss_back" }, { "empty", "empty" }, difficulty);
    };


    templates["notebook_health"] = [game](float difficulty) {
        return new Paper(game, { "notebook_health_front", "notebook_health_back" }, { "empty", "empty" }, difficulty);
    };

    templates["notebook_weapon"] = [game](float difficulty) {
        return new Paper(game, { "notebook_weapon_front", "notebook_weapon_back" }, { "empty", "empty" }, difficulty);
    };

    templates["grid_weapon"] = [game](float difficulty) {
        return new Paper(game, { "grid_weapon_front", "grid_weapon_back" }, { "empty", "empty" }, difficulty);
    };

    templates["notebook1"] = [game](float difficulty) {
        return new Paper(game, { "notebook1_front", "notebook1_back" }, { "notebook1_front", "notebook1_back" }, difficulty);
    };

    templates["notebook2"] = [game](float difficulty) {
        return new Paper(game, { "notebook2_front", "notebook2_back" }, { "notebook2_front", "notebook2_back" }, difficulty);
    };

    templates["notebook3"] = [game](float difficulty) {
        return new Paper(game, { "notebook3_front", "notebook3_back" }, { "notebook3_front", "notebook3_back" }, difficulty);
    };

    templates["notebook4"] = [game](float difficulty) {
        return new Paper(game, { "notebook4_front", "notebook4_back" }, { "notebook4_front", "notebook4_back" }, difficulty);
    };

    templates["notebook5"] = [game](float difficulty) {
        return new Paper(game, { "notebook5_front", "notebook5_back" }, { "notebook5_front", "notebook5_back" }, difficulty);
    };

    // Grid
    templates["grid1"] = [game](float difficulty) {
        return new Paper(game, { "grid1_front", "grid1_back" }, { "grid1_front", "grid1_back" }, difficulty);
    };

    templates["grid2"] = [game](float difficulty) {
        return new Paper(game, { "grid2_front", "grid2_back" }, { "grid2_front", "grid2_back" }, difficulty);
    };

    templates["grid3"] = [game](float difficulty) {
        return new Paper(game, { "grid3_front", "grid3_back" }, { "grid3_front", "grid3_back" }, difficulty);
    };

    templates["grid4"] = [game](float difficulty) {
        return new Paper(game, { "grid4_front", "grid4_back" }, { "grid4_front", "grid4_back" }, difficulty);
    };

    templates["grid5"] = [game](float difficulty) {
        return new Paper(game, { "grid5_front", "grid5_back" }, { "grid5_front", "grid5_back" }, difficulty);
    };

    // ---------------------
    // label templates
    // ---------------------

    // papers = {
    //     {SPAWN_ROOM, {
    //         "tutorial"
    //     }},
    //     {BASIC_ROOM, {
    //         "notebook1",
    //         "notebook2",
    //         "notebook3",
    //         "notebook4",
    //         "notebook5",
    //     }},
    //     {BOSS_ROOM, {
    //         "notebook_boss"
    //     }},
    //     {TREASURE_ROOM, {
    //         "notebook_weapon"
    //     }}
    // };

    // Include both notebook and grid templates - will be filtered by biome
    papers = {
        {SPAWN_ROOM, {
            "tutorial"
        }},
        {BASIC_ROOM, {
            "notebook1",
            "notebook2",
            "notebook3",
            "notebook4",
            "notebook5",
            "grid1",
            "grid2",
            "grid3",
            "grid4",
            "grid5",
        }},
        {BOSS_ROOM, {
            "notebook_boss",
            "grid_boss"
        }},
        {TREASURE_ROOM, {
            "notebook_weapon",
            "grid_weapon"
        }}
    };
}

Paper* Paper::getRandomTemplate(RoomTypes type, float difficulty, const std::string& biome) {
    // Filter templates by biome
    std::vector<std::string> filteredTemplates;
    for (const auto& templateName : papers[type]) {
        // For SPAWN_ROOM, always allow tutorial regardless of biome
        if (type == SPAWN_ROOM && templateName == "tutorial") {
            filteredTemplates.push_back(templateName);
        }
        // For other rooms, only allow if template name starts with biome prefix
        else if (templateName.find(biome) == 0) {
            filteredTemplates.push_back(templateName);
        }
    }
    
    // Fallback: if no templates match biome, use all templates (shouldn't happen)
    if (filteredTemplates.empty()) {
        filteredTemplates = papers[type];
    }
    
    uint numTemplates = filteredTemplates.size();
    uint index = randrange(0, numTemplates);
    return templates[filteredTemplates[index]](difficulty);
}

void Paper::flattenVertices(const std::vector<Vert>& vertices, std::vector<float>& data) {
    data.clear();
    data.reserve(vertices.size() * 5);

    for (const auto& v : vertices) {
        data.push_back(v.pos.x);
        data.push_back(v.pos.y);
        data.push_back(0.0f);
        data.push_back(v.uv.x);
        data.push_back(v.uv.y);
    }
}