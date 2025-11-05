#include "util/includes.h"
#include "game/game.h"
#include "levels/levels.h"

int main() {
    // create game
    Game* game = new Game();

    // add resources
    game->addImage("man", new Image("textures/man.png"));
    game->addImage("man", new Image("textures/container.jpg"));
    game->addMaterial("man", new Material({ 1, 1, 1 }, game->getImage("man")));
    game->addMesh("quad", new Mesh("models/quad.obj"));
    game->addCollider("quad", new Collider(game->getScene()->getSolver(), {{0.5, 0.5}, {-0.5, 0.5}, {-0.5, -0.5}, {0.5, -0.5}}));
    game->addCollider("ugh", new Collider(game->getScene()->getSolver(), {{0.5, 0.5}, {-1, 1}, {-0.5, -0.5}, {0.5, -0.5}}));

    // generate all templates
    SingleSide::generateTemplates(game);

    // create player
    Node2D* playerNode = new Node2D(game->getScene(), { .scale={1, 1}, .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .collider=game->getCollider("quad") });
    Player* player = new Player(3, 3, playerNode, nullptr);
    game->setPlayer(player);

    Node2D* enemyNode = new Node2D(game->getScene(), { .position={3, 4}, .scale={0.7, 0.7}, .mesh=game->getMesh("quad"), .material=game->getMaterial("man"), .collider=game->getCollider("quad") });
    game->addEnemy(new Enemy(3, 2, enemyNode, nullptr, nullptr));

    // Main loop continues as long as the window is open
    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 60);
    }

    // Free memory allocations
    delete game;
}