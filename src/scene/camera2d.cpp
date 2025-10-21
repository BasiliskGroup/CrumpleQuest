#include "scene/camera2d.h"

/**
 * @brief Construct a new Camera 2D object
 * 
 * @param position Starting position of the camera
 */
Camera2D::Camera2D(glm::vec2 position): position(position) {
    updateProjection();
    updateView();
}

/**
 * @brief Write the view and projection matrices to the given shader.
 *        Assumes uniform names are 'uView' and 'uProjection'
 * 
 * @param shader 
 */
void Camera2D::use(Shader* shader) {
    shader->setUniform("uView", view);
    shader->setUniform("uProjection", projection);
}

/**
 * @brief Handle inputs to update the camera
 * 
 * @param mouse 
 * @param keys 
 */
void Camera2D::update(Mouse* mouse, Keyboard* keys) {
    updateView();
}

/**
 * @brief Creates an orthigraphic projection fo the camera
 * 
 */
void Camera2D::updateProjection() {
    projection = glm::ortho(0.0f, 800.0f, 800.0f, 0.0f, -1.0f, 1.0f);
}

/**
 * @brief Updates the view matrix based on the current position
 * 
 */
void Camera2D::updateView() {
    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(position, 0.0f));
}