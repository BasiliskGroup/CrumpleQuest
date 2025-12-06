#include "pickup/ladder.h"
#include "levels/singleSide.h"

Ladder::Ladder(Game* game, SingleSide* side, Node2D::Params params, float radius) : Pickup(game, side, params, radius) {

}

void Ladder::onPickup() {
    std::cout << "Ladder picked up" << std::endl;
}