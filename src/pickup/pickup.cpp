#include "levels/levels.h"

Pickup::Pickup(Game* game, SingleSide* side, Node2D::Params node, float radius) : Node2D(side->getScene(), node), side(side), game(game), radius(radius) {
    
}

void Pickup::onPickup() {
    // Base implementation - should be overridden by derived classes
    // This default behavior adds health, but subclasses like Heart override this
    Player* player = game->getPlayer();
    if (player != nullptr) {
        player->addHealth(1);
    }
}