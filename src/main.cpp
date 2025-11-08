#include "util/includes.h"
#include "game/game.h"
#include "levels/levels.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    game->addImage("man", new Image("textures/man.png"));
    game->addImage("paper", new Image("textures/paper.png"));
    game->addImage("box", new Image("textures/container.jpg"));
    game->addMaterial("man", new Material({ 1, 1, 1 }, game->getImage("man")));
    game->addMaterial("paper", new Material({ 1, 1, 1 }, game->getImage("paper")));
    game->addMaterial("box", new Material({ 1, 1, 1 }, game->getImage("box")));
    game->addMesh("quad", new Mesh("models/quad.obj"));
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));
    game->addCollider("ugh", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-1.f, 1.f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));

    // create a piece of paper for a demo
    std::vector<std::vector<vec2>> paperDemoPolygon = {
        {{10.0, 10.0}, {40.0, 10.0}, {40.0, 30.0}, {10.0, 30.0}},
        {{20.0, 20.0}, {30.0, 20.0}, {30.0, 25.0}, {20.0, 25.0}},
        {{20.0, 12.5}, {30.0, 12.5}, {27.5, 15.0}}
    };
    for (uint i = 0; i < paperDemoPolygon.size(); i++) {
        for (uint j = 0; j < paperDemoPolygon.at(i).size(); j++) {
            paperDemoPolygon[i][j] *= 0.3;
            paperDemoPolygon[i][j] += vec2{-7.5, -6.5};
        }   
    }
    std::vector<uint> paperDemoIndices;
    std::vector<float> paperDemoData;
    Navmesh::earcut(paperDemoPolygon, paperDemoIndices);
    Navmesh::convertToMesh(paperDemoPolygon, paperDemoIndices, paperDemoData);
    game->addMesh("paper", new Mesh(paperDemoData));

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .scale={1, 1}, .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    Node2D* enemyNode = new Node2D(game->getScene(), { .position={3, 4}, .scale={0.7, 0.7}, .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .collider=game->getCollider("quad") });
    game->addEnemy(new Enemy(3, 2, enemyNode, nullptr, nullptr));

    // polygon clip test
    Clipper2Lib::PathsD subject = {{{0,0}, {100,0}, {100,100}, {0,100}}};
    Clipper2Lib::PathsD clip = {{{50,-50}, {150,-50}, {150,50}, {50,50}}};

    Clipper2Lib::PathsD result = Clipper2Lib::Intersect(subject, clip, Clipper2Lib::FillRule::NonZero);

    std::cout << "Intersection result count: " << result.size() << "\n";

    // Loop through each path (polygon)
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "Polygon " << i << ":\n";
        for (const Clipper2Lib::PointD& p : result[i]) {
            std::cout << "  (" << p.x << ", " << p.y << ")\n";
        }
    }

    // background paper
    // Node2D* paper = new Node2D(game->getScene(), { .mesh=game->getMesh("paper"), .material=game->getMaterial("paper") });

    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 120);
    }

    delete game;
}