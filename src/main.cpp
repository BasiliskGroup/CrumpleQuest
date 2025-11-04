#include "util/includes.h"
#include "game/game.h"

int main() {
    Game* game = new Game();

    // Main loop continues as long as the window is open
    while (game->getEngine()->isRunning()) {
        game->update(1.0 / 60);
    }

    // Free memory allocations
    delete game;
}