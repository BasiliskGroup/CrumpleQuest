#include "levels/levels.h"
#include "pickup/heart.h"

Heart::Heart(Game* game, SingleSide* side, Node2D::Params node, float radius) : Pickup(game, side, node, radius) {
    
}

void Heart::update(float dt) {
    
}

void Heart::onPickup() {
    game->getPlayer()->addHealth(1);
}

bool Heart::canPickup() {
    return game->getPlayer()->getHealth() < game->getPlayer()->getMaxHealth();
}
