#ifndef HEART_H
#define HEART_H

#include "pickup/pickup.h"
#include "resource/animation.h"
#include "resource/animator.h"

class Game;
class Player;

class Heart : public Pickup {
private:
    Animator* animator;
    Animation* animation;

public:
    Heart(Game* game, SingleSide* side, Node2D::Params node, float radius);
    ~Heart();

    void onPickup() override;
    bool canPickup(Player* player) override;
    void update(float dt) override;
};

#endif