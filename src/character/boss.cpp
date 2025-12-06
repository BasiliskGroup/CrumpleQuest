#include "character/boss.h"

Boss::Boss(Game* game, Node2D* node, SingleSide* side, Weapon* weapon, float radius, vec2 scale, vec2 hitboxOffset) : Character(game, 100, 1, node, side, weapon, "Boss", radius, scale, "hit-boss") {
    this->hitboxOffset = hitboxOffset;
    this->stage = "attack1";
}

Boss::~Boss() {

}