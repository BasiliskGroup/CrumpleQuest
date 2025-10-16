#ifndef MOVEMENT_H
#define MOVEMENT_H

class Movement {
protected:
    float speed;

public:
    Movement() = default;
    ~Movement() = default;

    virtual void move() = 0;
};

#endif