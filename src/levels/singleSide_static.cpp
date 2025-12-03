#include "levels/levels.h"
#include "character/enemy.h"


std::unordered_map<std::string, std::function<SingleSide*()>> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {

    // empty level
    templates["empty0"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper0", "paper");
        side->addEnemy(Enemy::templates["clipfly"]({ -5, 3 }, side));
        side->addEnemy(Enemy::templates["glue"]({ 0, 3 }, side));
        side->addEnemy(Enemy::templates["staple"]({ 5, 3 }, side));
        return side;
    };

    templates["empty1"] = [game]() {
        SingleSide* side = new SingleSide(game, "paper1", "paper");
        return side;
    };
}