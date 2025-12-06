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
        return side;
    };
    templates["tutorial_back"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper1", "tutorial_tutorial", vec2(0, 0), "notebook", {}, difficulty);
        side->addEnemy(Enemy::templates["clipfly"]({ 10.4 - 6.0, 4.43 - 4.5 }, side));
        return side;
    };

    // Notebook
    templates["notebook_health_front"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper0", "notebook_blank", vec2(0, 0), "notebook", {}, difficulty);
        side->addPickup(new Heart(game, side, { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position={2.5, 0.0}, .scale={1.0, 1.0} }, 0.5f));
        side->addPickup(new Heart(game, side, { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position={-2.5, 0.0}, .scale={1.0, 1.0} }, 0.5f));
        return side;
    };
    templates["notebook_health_back"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper1", "notebook_blank", vec2(0, 0), "notebook", {}, difficulty);
        return side;
    };

    templates["notebook_weapon_front"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper0", "notebook_blank", vec2(0, 0), "notebook", {}, difficulty);
        side->addPickup(new StapleGun(game, side, { .mesh=game->getMesh("quad"), .material=game->getMaterial("empty"), .position={-3, 0.0}, .scale={1.0, 1.0} }, 0.5f));
        return side;
    };
    templates["notebook_weapon_back"] = [game](float difficulty) {
        SingleSide* side = new SingleSide(game, "paper1", "notebook_blank", vec2(0, 0), "notebook", {}, difficulty);
        return side;
    };

    templates["notebook1_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {10.4,4.43} , {1.54,4.36} };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook1_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {4.8,5.4}, {6.93,3.17}, {10.1,4.38} };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook2_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {1.1,4.35}, {10.78,4.3}, {6.22,8.12} };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook2_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {9.04,6.27}, {2.84,3.15}, {2.8,6.1} };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook3_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {8.7,4.8}, {5.32,2.4} };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level3", vec2(3.17,4.56), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook3_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {1.66,6.5}, {6.02,4.32}, {9.34,0.85} };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level3", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook4_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {7.47,7.93}, {4.06,7.9}, {4.2,1.16}, {7.78,1.06} };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook4_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {5.96,4.43}, {1.4,2.33}, {10.95,7.18}};
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook5_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {3.7,7.53}, {8.16,1.35}};
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level5", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook5_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { {5.87,7.4}, {5.84,2.3}, {9.34,4.8}, {2.32,4.67} };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level5", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    // templates["grid1_front"] = [game](float difficulty) {
    //     std::vector<vec2> enemySpawns = { { 10,4.5 }, { 1.47,4.67 } };
    //     SingleSide* side = new SingleSide(game, "paper0", "grid_level1", vec2(0, 0), "grid", enemySpawns, difficulty);
    //     return side;
    // };
    // templates["grid1_back"] = [game](float difficulty) {
    //     std::vector<vec2> enemySpawns = { { 1.44,1.7 }, { 10.13,7.25 }, { 6.03,4.42 }, { 10.3,1.77 }, { 1.74,7.47 } };
    //     SingleSide* side = new SingleSide(game, "paper1", "grid_level1", vec2(0, 0), "grid", enemySpawns, difficulty);
    //     return side;
    // };
}

Node2D* SingleSide::genPlayerNode(Game* game, SingleSide* side) {
    Node2D* node = new Node2D(side->getScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("empty"),
        .position = { 0, -3 },
        .rotation = 0,
        .scale = { 1.5, 1.5 },
        .collider = side->getCollider("quad"),
        .colliderScale = { 0.5, 0.7 }
    });
    node->setManifoldMask(1, 1, 0);
    return node;
}