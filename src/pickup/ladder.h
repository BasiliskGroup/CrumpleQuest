#ifndef LADDER_H
#define LADDER_H

#include "pickup/pickup.h"

class Ladder : public Pickup {
public:
    Ladder(Game* game, SingleSide* side, Node2D::Params params, float radius);
    ~Ladder() = default;

    bool canPickup(Player* player) override { return true; }
    void onPickup() override;
};

#endif