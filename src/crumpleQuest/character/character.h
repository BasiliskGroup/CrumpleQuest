#ifndef CHARACTER_H
#define CHARACTER_H

class Weapon;

class Character {
protected:
    int health;
    Weapon* weapon;

public:
    Character(int health, Weapon* weapon);
    ~Character() = default;

    void onDamage(int damage);

    // virtual methods
    virtual void move() = 0;

private:
    void clear();
};

#endif