#ifndef PICKUP_STAPLEGUN_H
#define PICKUP_STAPLEGUN_H

#include "pickup/pickup.h"
#include "resource/animation.h"
#include "resource/animator.h"

class Game;
class Player;

class StapleGun : public Pickup {
private:
    Animator* animator;
    Animation* animation;

public:
    StapleGun(Game* game, SingleSide* side, Node2D::Params node, float radius);
    ~StapleGun();

    void onPickup() override;
    bool canPickup(Player* player) override;
    void update(float dt) override;
};

#endif