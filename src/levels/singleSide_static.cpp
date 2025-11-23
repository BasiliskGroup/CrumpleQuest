#include "levels/levels.h"

std::unordered_map<std::string, std::function<SingleSide*()>> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {

    // empty level
    templates["empty"] = [game]() {
        SingleSide* side = new SingleSide(game);
        return side;
    };
}