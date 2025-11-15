#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    // image
    game->addImage("man", new Image("textures/man.png"));
    game->addImage("paper", new Image("textures/paper.png"));
    game->addImage("box", new Image("textures/container.jpg"));
    game->addImage("floor", new Image("textures/floor_albedo.png"));

    // material
    game->addMaterial("man", new Material({ 1, 1, 1 }, game->getImage("man")));
    game->addMaterial("paper", new Material({ 1, 1, 1 }, game->getImage("paper")));
    game->addMaterial("box", new Material({ 1, 1, 1 }, game->getImage("box")));
    game->addMaterial("floor", new Material({ 1, 1, 1 }, game->getImage("floor")));

    // mesh
    game->addMesh("quad", new Mesh("models/quad.obj"));
    game->addMesh("paper", new Mesh("models/paper.obj"));

    // collider
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));
    game->addCollider("ugh", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-1.f, 1.f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .scale={1, 1}, .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    // test add enemy
    Node2D* enemyNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position={3, 4}, .scale={0.7, 0.7}, .collider=game->getCollider("quad") });
    game->addEnemy(new Enemy(3, 0.1, enemyNode, nullptr, nullptr));

    // test add button
    Button* testButton = new Button(game, { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .position={-2, -2}, .scale={0.5, 0.5} }, { 
        .onDown=[]() { std::cout << "Button Pressed" << std::endl; },
        .onUp=[]() { std::cout << "Button Released" << std::endl; }
    });
    game->addButton(testButton);

    // create test paper
    game->setPaper(new Paper(
        game->getMesh("paper"), 
        {{2.0, 1.5}, {-2.0, 1.5}, {-2.0, -1.5}, {2.0, -1.5}}
    ));

    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 120);
    }

    delete game;
}