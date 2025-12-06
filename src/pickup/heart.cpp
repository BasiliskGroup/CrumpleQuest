#include "levels/levels.h"
#include "pickup/heart.h"

Heart::Heart(Game* game, SingleSide* side, Node2D::Params node, float radius) : Pickup(game, side, node, radius) {
    
}

void Heart::update(float dt) {

}

void Heart::onPickup() {
    Player* player = game->getPlayer();
    if (player != nullptr) {
        player->addHealth(1);
    }
}

bool Heart::canPickup(Player* player) {
    if (player == nullptr) return false;
    return player->getHealth() < player->getMaxHealth();
}
