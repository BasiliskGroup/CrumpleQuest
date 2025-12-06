#ifndef PICKUP_H
#define PICKUP_H

#include "util/includes.h"

class SingleSide;
class Game;
class Player;

class Pickup : public Node2D {
protected:
    float radius;
    SingleSide* side;
    Game* game;

public:
    Pickup(Game* game, SingleSide* side, Node2D::Params node, float radius);
    ~Pickup() = default;

    virtual void update(float dt) {}

    virtual bool canPickup(Player* player) { return true; }

    float getRadius() const { return radius; }
    virtual void onPickup();
};

#endif