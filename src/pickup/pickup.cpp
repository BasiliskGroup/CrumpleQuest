#include "levels/levels.h"

Pickup::Pickup(Game* game, SingleSide* side, Node2D::Params node, float radius) : Node2D(side->getScene(), node), side(side), game(game), radius(radius) {
    
}

void Pickup::onPickup() {
    game->getPlayer()->addHealth(1);
}