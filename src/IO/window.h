#ifndef WINDOW_H
#define WINDOW_H

#include "util/includes.h"

class Window {
    private:
        GLFWwindow* window;
        unsigned int width;
        unsigned int height;

    public:
        Window(int width, int height, const char* title);
        ~Window();

        void resize(GLFWwindow* window, int width, int height);

        bool isRunning();
        void render();
        void clear(float r=0.0, float g=0.0, float b=0.0, float a=1.0);

        GLFWwindow* getWindow() { return window; }
};

#endif