#ifndef CHARACTER_H
#define CHARACTER_H

class Weapon;

class Character {
private:
    int health;
    Weapon* weapon;

public:
    Character(int health, Weapon* weapon);

    // virtual methods
    virtual void onDamage(int damage) = 0;
    
};

#endif