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

    templates["notebook_health"] = [game](float difficulty) {
        return new Paper(game, { "notebook_health_front", "notebook_health_back" }, { "empty", "empty" }, difficulty);
    };

    templates["notebook_weapon"] = [game](float difficulty) {
        return new Paper(game, { "notebook_weapon_front", "notebook_weapon_back" }, { "empty", "empty" }, difficulty);
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

    papers = {
        {SPAWN_ROOM, {
            "tutorial"
        }},
        {BASIC_ROOM, {
            // "notebook1",
            // "notebook2",
            // "notebook3",
            // "notebook4",
            // "notebook5",
            // "notebook_health",
            "grid1",
            "grid2",
            "grid3",
            "grid4",
            "grid5",
        }},
        {BOSS_ROOM, {
            "tutorial"
        }},
        {TREASURE_ROOM, {
            "notebook_weapon"
        }}
    };
}

Paper* Paper::getRandomTemplate(RoomTypes type, float difficulty) {
    uint numTemplates = papers[type].size();
    uint index = randrange(0, numTemplates);
    return templates[papers[type][index]](difficulty);
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