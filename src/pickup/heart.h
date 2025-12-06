#ifndef HEART_H
#define HEART_H

#include "pickup/pickup.h"

class Game;

class Heart : public Pickup {
public:
    Heart(Game* game, SingleSide* side, Node2D::Params node, float radius);
    ~Heart() = default;

    void onPickup() override;
    bool canPickup() override;
    void update(float dt) override;
};

#endif