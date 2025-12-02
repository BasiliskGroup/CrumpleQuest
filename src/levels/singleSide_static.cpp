#include "levels/levels.h"

std::unordered_map<std::string, std::function<SingleSide*()>> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {

    // empty level
    templates["empty0"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper0", "test");
        return side;
    };

    templates["empty1"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper1", "test");
        return side;
    };
}