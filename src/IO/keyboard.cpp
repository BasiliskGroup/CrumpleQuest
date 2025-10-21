#include "IO/keyboard.h"

bool Keyboard::getPressed(unsigned int keyCode) {
    return (glfwGetKey(window->getWindow(), keyCode) == GLFW_PRESS);
}