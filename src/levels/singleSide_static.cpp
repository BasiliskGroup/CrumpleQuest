#include "levels/levels.h"
#include "character/enemy.h"
#include "pickup/heart.h"
#include "pickup/stapleGun.h"
#include "pickup/scissor.h"


std::unordered_map<std::string, std::function<SingleSide*(float)>> SingleSide::templates;

void SingleSide::generateTemplates(Game* game) {

    // Tutorial
    templates["tutorial_front"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper0", "tutorial_tutorial", vec2(0, 0), "notebook", {}, difficulty);

        side->addPickup(new StapleGun(game, side, { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position={2.0 - 6.0, 2.0 - 4.5}, .scale={1.0, 1.0} }, 0.5f));

        return side;
    };
    templates["tutorial_back"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper1", "tutorial_tutorial", vec2(0, 0), "notebook", {}, difficulty);
        side->addEnemy(Enemy::templates["clipfly"]({ 10.4 - 6.0, 4.43 - 4.5 }, side));
        return side;
    };

    // Notebook
    templates["notebook1_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 10,4.5 }, { 1.47,4.67 } };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook1_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 1.44,1.7 }, { 10.13,7.25 }, { 6.03,4.42 }, { 10.3,1.77 }, { 1.74,7.47 } };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook2_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 1.5,3.7 }, { 3.5,7.95 }, { 7.22,4.6 } };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook2_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 2.37,1.85 }, { 4.97,6.2 }, { 9.35,1.63 }, { 10.26,7.22 } };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook3_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 1.2,1.33 }, { 1.22,7.64 }, { 10.5,1.28 } };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level3", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook3_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 8.6,4.6 }, { 5.9,6.03 }, { 2.24,4.4 }, { 6.62,1.6 }, { 2.77,0.98 } };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level3", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook4_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 7.9,1.3 }, { 1.4,6.97 }, { 10.56,3.08 } };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook4_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 2.27,0.88 }, { 6.16,7.84 }, { 10.5,4.37 } };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook5_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 0.7,1.5 }, { 5.67,6.86 }, { 11,7.45 } };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level5", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook5_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { { 4.58,4.33 }, { 7.17,4.37 }, { 10,8.15 }, { 3.85,0.9 } };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level5", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

}

Node2D* SingleSide::genPlayerNode(Game* game, SingleSide* side) {
    Node2D* node = new Node2D(side->getScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("empty"),
        .position = { 0, -3 },
        .rotation = 0,
        .scale = { 1.5, 1.5 },
        .collider = side->getCollider("quad"),
        .colliderScale = { 0.5, 0.8 }
    });
    node->setManifoldMask(1, 1, 0);
    return node;
}