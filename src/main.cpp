#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    // image and material
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey" };
    for (std::string& name : imageNames) {
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper" };
    for (std::string& name : meshNames) game->addMesh(name, new Mesh("models/" + name + ".obj"));

    // collider
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5f, 0.5f}, {-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f}}));

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .scale={1, 1}, .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    // test add button
    Button* testButton = new Button(game, { .mesh=game->getMesh("quad"), .material=game->getMaterial("box"), .position={-2, -2}, .scale={0.5, 0.5} }, { 
        .onDown=[]() { std::cout << "Button Pressed" << std::endl; }
    });

    // spawn enemy on click
    testButton->setOnUp([game]() {
        Node2D* enemyNode = new Node2D(game->getScene(), { .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .position={3, 4}, .scale={0.7, 0.7}, .collider=game->getCollider("quad") });
        game->addEnemy(new Enemy(3, 0.1, enemyNode, nullptr, nullptr));
    });

    game->addUI(testButton);

    // slider
    Slider* testSlider = new Slider(game, { -4, 4 }, { 0, 4 }, { .pegMaterial=game->getMaterial("box") });
    testSlider->setCallback([game](float proportion) {
        for (Enemy* enemy : game->getEnemies()) {
            enemy->setSpeed(5 * proportion);
        }
    });
    game->addUI(testSlider);

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