#include "pickup/ladder.h"
#include "levels/singleSide.h"
#include "game/game.h"

Ladder::Ladder(Game* game, SingleSide* side, Node2D::Params params, float radius) : Pickup(game, side, params, radius) {

}

void Ladder::onPickup() {
    std::cout << "Ladder picked up - requesting floor reset" << std::endl;
    if (game != nullptr) {
        game->requestResetFloor();
    }
}