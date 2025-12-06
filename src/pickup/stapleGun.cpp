#include "levels/levels.h"
#include "pickup/stapleGun.h"

StapleGun::StapleGun(Game* game, SingleSide* side, Node2D::Params node, float radius) : Pickup(game, side, node, radius) {
    animation = game->getAnimation("integral_idle");
    animator = new Animator(game->getEngine(), this, animation);
    animator->setFrameRate(8);
}

StapleGun::~StapleGun() {
    delete animator;
}

void StapleGun::update(float dt) {
    animator->update();
}

void StapleGun::onPickup() {
    Player* player = game->getPlayer();
    if (player != nullptr) {
        player->setStapleGunAvailible();
        player->setWeaponStapleGun();
    }
}

bool StapleGun::canPickup(Player* player) {
    return true;
}
