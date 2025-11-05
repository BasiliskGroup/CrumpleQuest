#include "levels/levels.h"

std::unordered_map<std::string, SingleSide> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {
    Scene2D* voidScene = game->getVoidScene();

    templates["empty"] = SingleSide();
}