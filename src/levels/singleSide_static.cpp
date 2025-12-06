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
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook1_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level1", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook2_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook2_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level2", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook3_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level3", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook3_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level3", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook4_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook4_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper1", "notebook_level4", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };

    templates["notebook5_front"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
        SingleSide* side = new SingleSide(game, "paper0", "notebook_level5", vec2(0, 0), "notebook", enemySpawns, difficulty);
        return side;
    };
    templates["notebook5_back"] = [game](float difficulty) {
        std::vector<vec2> enemySpawns = { };
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
        .colliderScale = { 0.5, 0.8 }
    });
    node->setManifoldMask(1, 1, 0);
    return node;
}