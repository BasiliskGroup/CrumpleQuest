#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "util/includes.h"
#include "IO/window.h"

class Keyboard {
    private:
        Window* window;

    public:
        Keyboard(Window* window): window(window) {}

        bool getPressed(unsigned int keyCode);
};

#endif