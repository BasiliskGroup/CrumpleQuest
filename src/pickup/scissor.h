#ifndef PICKUP_SCISSOR_H
#define PICKUP_SCISSOR_H

#include "pickup/pickup.h"
#include "resource/animation.h"
#include "resource/animator.h"

class Game;
class Player;

class Scissor : public Pickup {
private:
    Animator* animator;
    Animation* animation;

public:
    Scissor(Game* game, SingleSide* side, Node2D::Params node, float radius);
    ~Scissor();

    void onPickup() override;
    bool canPickup(Player* player) override;
    void update(float dt) override;
};

#endif