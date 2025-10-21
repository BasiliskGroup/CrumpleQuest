#include "scene/camera.h"


/**
 * @brief Construct a new Camera object
 * 
 * @param position 
 * @param pitch 
 * @param yaw 
 */
Camera::Camera(glm::vec3 position, float pitch, float yaw): 
    position(position), pitch(pitch), yaw(yaw) 
{
    updateView();
    updateProjection();
}

void Camera::update(Mouse* mouse, Keyboard* keys) {

    // Looking
    float yOffset = mouse->getRelativeY() * sensitivity / 5;
    float xOffset = mouse->getRelativeX() * sensitivity / 5;

    yaw += xOffset;
    pitch -= yOffset;
    pitch = std::max(-89.0f, std::min(89.0f, pitch));

    // Movement
    float speed = 3.0;
    float dt = 0.005;
    float velocity = (speed * dt) * (keys->getPressed(GLFW_KEY_CAPS_LOCK) * 3 + 1);

    moveForward((keys->getPressed(GLFW_KEY_W) - keys->getPressed(GLFW_KEY_S)) * velocity);
    moveSide((keys->getPressed(GLFW_KEY_D) - keys->getPressed(GLFW_KEY_A)) * velocity);
    moveUp((keys->getPressed(GLFW_KEY_SPACE) - keys->getPressed(GLFW_KEY_LEFT_SHIFT)) * velocity);

    updateProjection();
    updateView();
}

/**
 * @brief 
 * 
 */
void Camera::updateView() {
    forward = {
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    right = glm::normalize(glm::cross(worldUp, forward));
    up = glm::cross(forward, right);

    view = glm::lookAt(position, position + forward, up);
}

/**
 * @brief Update the projection matrix based on current parameters
 * 
 */
void Camera::updateProjection() {
    projection = glm::perspective(fov, aspect, near, far);
}

/**
 * @brief Write the view and projection matrices to the given shader.
 *        Assumes uniform names are 'uView' and 'uProjection'
 * 
 * @param shader 
 */
void Camera::use(Shader* shader) {
    shader->setUniform("uView", view);
    shader->setUniform("uProjection", projection);
}

void Camera::moveSide(float distance) {
    position -= right * distance;
}

void Camera::moveForward(float distance) {
    position += glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * distance;
}

void Camera::moveUp(float distance) {
    position.y += distance;
}