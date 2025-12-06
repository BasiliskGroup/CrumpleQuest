#include "levels/levels.h"
#include "pickup/stapleGun.h"

StapleGun::StapleGun(Game* game, SingleSide* side, Node2D::Params node, float radius) : Pickup(game, side, node, radius) {
    this->setMaterial(game->getMaterial("stapleGunPickup"));
}

StapleGun::~StapleGun() {

}

void StapleGun::update(float dt) {
    
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
