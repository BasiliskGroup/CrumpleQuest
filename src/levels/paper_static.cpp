#include "levels/levels.h"
#include "util/random.h"

std::unordered_map<std::string, Paper> Paper::templates;
std::unordered_map<RoomTypes, std::vector<std::string>> Paper::papers;

void Paper::generateTemplates(Game* game) {
    Scene2D* voidScene = game->getVoidScene();

    // ---------------------
    // create templates
    // ---------------------

    templates["empty"] = Paper(game->getMesh("paper0"), game->getMesh("paper1"), {{2.0, 1.5}, {-2.0, 1.5}, {-2.0, -1.5}, {2.0, -1.5}});

    // ---------------------
    // label templates
    // ---------------------

    papers = {
        {SPAWN_ROOM, {
            "empty"
        }},
        {BASIC_ROOM, {

        }},
        {BOSS_ROOM, {

        }}
    };
}

const Paper& Paper::getRandomTemplate(RoomTypes type) {
    uint numTemplates = papers[type].size();
    uint index = randrange(0, numTemplates);
    return templates[papers[type][index]];
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