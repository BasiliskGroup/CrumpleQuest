#ifndef CHARACTER_H
#define CHARACTER_H

#include "util/includes.h"

class SingleSide;
class Weapon;
class Game;
struct PaperMesh;

class Character {
protected:
    int maxHealth;
    int health;
    float speed;
    float accel = 5;
    float itime = 0;
    float radius = 0.25;
    vec2 moveDir = vec2();
    Weapon* weapon;
    Node2D* node;
    std::string team;
    vec2 scale;
    std::string damageSound;

    // back reference pointers
    SingleSide* side;
    Game* game;

public:
    Character(Game* game, int maxHealth, float speed, Node2D* node, SingleSide* side, Weapon* weapon, std::string team, float radius, vec2 scale, std::string damageSound);
    ~Character();

    virtual void onDamage(int damage);
    virtual void onDeath();

    bool isDead() { return health <= 0; }

    // getters
    int& getMaxHealth() { return maxHealth; }
    int& getHealth() { return health; }
    float& getSpeed() { return speed; }
    Weapon*& getWeapon() { return weapon; }
    std::string getTeam() { return team; }
    Node2D* getNode() { return node; }
    SingleSide* getSide() { return side; }
    Game* getGame() { return game; }

    vec2 getPosition() { return node->getPosition(); }
    vec3 getVelocity() { return node->getVelocity(); }
    float getRadius() { return radius; }
    vec2& getMoveDir() { return moveDir; }
    std::string getDamageSound() { return damageSound; }

    // setters
    void setMaxMealth(int maxHealth) { this->maxHealth = maxHealth; }
    void setHealth(int health) { this->health = health; }
    void setSpeed(float speed) { this->speed = speed; }
    void setWeapon(Weapon* weapon) { this->weapon = weapon; }
    void setTeam(std::string team) { this->team = team; }

    void setVelocity(const vec3& velocity) { this->node->setVelocity(velocity); }
    void setPosition(const vec2& position) { this->node->setPosition(position); }
    void setNode(Node2D* node) { this->node = node; }
    void setSide(SingleSide* side) { this->side = side; }

    void move(float dt);
    
    // Line of sight helper
    bool hasLineOfSight(const vec2& start, const vec2& end) const;
    
    // Get PaperMesh for this character's side
    PaperMesh* getPaperMeshForSide() const;
};

#endif