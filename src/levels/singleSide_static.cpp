#include "levels/levels.h"
#include "character/enemy.h"


std::unordered_map<std::string, std::function<SingleSide*()>> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {

    // empty level
    templates["empty0"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper0", "knight");
        side->addEnemy(Enemy::templates["clipfly"]({ -5, 3 }, side));
        return side;
    };

    templates["empty1"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper1", "knight");
        return side;
    };
}