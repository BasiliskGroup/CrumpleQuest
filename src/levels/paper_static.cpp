#include "levels/levels.h"
#include "util/random.h"

std::unordered_map<std::string, Paper> Paper::templates;
std::unordered_map<RoomTypes, std::vector<std::string>> Paper::papers;

void Paper::generateTemplates(Game* game) {
    Scene2D* voidScene = game->getVoidScene();

    // ---------------------
    // create templates
    // ---------------------

    templates["empty"] = Paper(
        new SingleSide(SingleSide::templates["empty"]),
        new SingleSide(SingleSide::templates["empty"])
    );

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