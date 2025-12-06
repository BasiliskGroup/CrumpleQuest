#include "levels/levels.h"
#include "pickup/scissor.h"

Scissor::Scissor(Game* game, SingleSide* side, Node2D::Params node, float radius) : Pickup(game, side, node, radius) {
    animation = game->getAnimation("pi_idle");
    animator = new Animator(game->getEngine(), this, animation);
    animator->setFrameRate(8);
}

Scissor::~Scissor() {
    delete animator;
}

void Scissor::update(float dt) {
    animator->update();
}

void Scissor::onPickup() {
    Player* player = game->getPlayer();
    if (player != nullptr) {
        player->setScissorAvailible();
        player->setWeaponScissor();
    }
}

bool Scissor::canPickup(Player* player) {
    return true;
}
