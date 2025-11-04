#ifndef CHARACTER_H
#define CHARACTER_H

#include <basilisk/basilisk.h>

class Weapon;

class Character {
protected:
    int maxHealth;
    int health;
    float speed;
    Weapon* weapon;

public:
    Character(int maxHealth, Weapon* weapon);
    ~Character() = default; // dont delete weapon

    void onDamage(int damage);
    void onDeath();

    bool isDead() { return health < 0; }

    // getters
    int& getMaxHealth() { return maxHealth; }
    int& getHealth() { return health; }
    float& getSpeed() { return speed; }
    Weapon*& getWeapon() { return weapon; }

    // setters
    void setMaxMealth(int maxHealth) { this->maxHealth = maxHealth; }
    void setHealth(int health) { this->health = health; }
    void setSpeed(float speed) { this->speed = speed; }
    void setWeapon(Weapon* weapon) { this->weapon = weapon; }

    // virtual methods
    virtual void move() = 0;
};

#endif