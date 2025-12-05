#include "levels/levels.h"
#include "util/random.h"

std::unordered_map<std::string, std::function<Paper*()>> Paper::templates;
std::unordered_map<RoomTypes, std::vector<std::string>> Paper::papers;

void Paper::generateTemplates(Game* game) {
    // ---------------------
    // create templates
    // ---------------------

    templates["empty"] = [game]() {
        return new Paper(game->getMesh("paper0"), game->getMesh("paper1"), 
        {vec2{ 6.0,  4.5},vec2{-6.0,  4.5},vec2{-6.0, -4.5},vec2{ 6.0, -4.5} }, 
        { "empty0", "empty1" }, { "empty", "empty" });
    };

    templates["squareMiddle"] = [game]() {
        return new Paper(game->getMesh("paper0"), game->getMesh("paper1"), 
        {vec2{ 6.0,  4.5},vec2{-6.0,  4.5},vec2{-6.0, -4.5},vec2{ 6.0, -4.5} }, 
        { "empty0", "empty1" }, { "squareMiddle", "squareMiddle" });
    };

    // ---------------------
    // label templates
    // ---------------------

    papers = {
        {SPAWN_ROOM, {
            "empty"
        }},
        {BASIC_ROOM, {
            "squareMiddle"
        }},
        {BOSS_ROOM, {
            "squareMiddle"
        }}
    };
}

Paper* Paper::getRandomTemplate(RoomTypes type) {
    uint numTemplates = papers[type].size();
    uint index = randrange(0, numTemplates);
    return templates[papers[type][index]]();
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