#ifndef CHARACTER_H
#define CHARACTER_H

#include "util/includes.h"

class SingleSide;
class Weapon;

class Character {
protected:
    int maxHealth;
    int health;
    float speed;
    float accel = 30;
    float itime = 0;
    float radius = 0.5;
    vec2 moveDir = vec2();
    Weapon* weapon;
    Node2D* node;
    SingleSide* side;
    std::string team;

public:
    Character(int maxHealth, float speed, Node2D* node, SingleSide* side, Weapon* weapon, std::string team);
    ~Character();

    void onDamage(int damage);
    void onDeath();

    bool isDead() { return health < 0; }

    // getters
    int& getMaxHealth() { return maxHealth; }
    int& getHealth() { return health; }
    float& getSpeed() { return speed; }
    Weapon*& getWeapon() { return weapon; }
    std::string getTeam() { return team; }
    Node2D* getNode() { return node; }
    SingleSide* getSide() { return side; }

    vec2 getPosition() { return node->getPosition(); }
    vec3 getVelocity() { return node->getVelocity(); }
    float getRadius() { return radius; }

    // setters
    void setMaxMealth(int maxHealth) { this->maxHealth = maxHealth; }
    void setHealth(int health) { this->health = health; }
    void setSpeed(float speed) { this->speed = speed; }
    void setWeapon(Weapon* weapon) { this->weapon = weapon; }
    void setTeam(std::string team) { this->team = team; }

    void setVelocity(const vec3& velocity) { this->node->setVelocity(velocity); }
    void setPosition(const vec2& position) { this->node->setPosition(position); }

    void move(float dt);
};

#endif