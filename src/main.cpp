#include "util/includes.h"
#include "game/game.h"

int main() {
    // create game
    Game* game = new Game();

    // add resources
    game->addImage("man", new Image("textures/man.png"));
    game->addMaterial("man", new Material({ 1, 1, 1 }, game->getImage("man")));
    game->addMesh("quad", new Mesh("models/quad.obj"));
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}}));
    game->addCollider("ugh", new Collider(game->getScene()->getSolver(), {{0.5, 0.5}, {-1, 1}, {-0.5, -0.5}, {0.5, -0.5}}));

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .scale={1, 1}, .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .collider=game->getCollider("quad"), .velocity={1, 1, 0}});
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    // Main loop continues as long as the window is open
    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 60);
    }

    // Free memory allocations
    delete game;
}