#include "IO/mouse.h"

/**
 * @brief Update the state of the mouse and store previous state
 * 
 */
void Mouse::update() {
    // Store the previous frame's mouse state
    previousX = x;
    previousY = y;
    
    previousLeft   = left;
    previousMiddle = middle;
    previousRight  = right;
    
    // Update the position and click states
    glfwGetCursorPos(window->getWindow(), &x, &y);

    left = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    middle = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    right = glfwGetMouseButton(window->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
}

/**
 * @brief Grabs the mouse and hides it
 * 
 */
void Mouse::setGrab() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

/**S
 * @brief Shows the mouse and ungrabs it
 * 
 */
void Mouse::setVisible() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

/**
 * @brief Hides the mouse but does not grab it
 * 
 */
void Mouse::setHidden() {
    glfwSetInputMode(window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}