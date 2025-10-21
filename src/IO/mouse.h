#ifndef MOUSE_H
#define MOUSE_H

#include "util/includes.h"
#include "IO/window.h"

class Mouse {
    private:
        Window* window;
        double x, y;
        double previousX, previousY;

        bool left, middle, right;
        bool previousLeft;
        bool previousMiddle;
        bool previousRight;
    
    public:
        Mouse(Window* window): window(window), x(400), y(300), previousX(400), previousY(300) {}
        
        void update();

        bool getClicked()       { return left && !previousLeft; }
        bool getMiddleClicked() { return middle && !previousMiddle; }
        bool getRightClicked()  { return right && !previousRight; }

        bool getLeftDown()   { return left; }
        bool getMiddleDown() { return middle; }
        bool getRightDown()  { return right; }

        double getX() { return x; }
        double getY() { return y; }

        double getRelativeX() { return x - previousX; }
        double getRelativeY() { return y - previousY; }

        void setGrab();
        void setVisible();
        void setHidden();
};

#endif