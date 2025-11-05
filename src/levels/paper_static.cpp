#include "levels/levels.h"

std::unordered_map<std::string, Paper> Paper::templates;

void Paper::generateTemplates(Game* game) {
    Scene2D* voidScene = game->getVoidScene();

    templates["empty"] = Paper(
        new SingleSide(SingleSide::templates["empty"]),
        new SingleSide(SingleSide::templates["empty"])
    );
}