/**
 * @file main.cpp
 * @author Jonah Coffelt
 * @brief ...
 * @version 0.1
 * @date 2025-10-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "IO/window.h"
#include "render/shader.h"
#include "render/vbo.h"
#include "render/ebo.h"
#include "render/vao.h"
#include "render/image.h"
#include "render/texture.h"
#include "scene/camera.h"
#include "IO/mouse.h"
#include "IO/keyboard.h"
#include "render/mesh.h"
#include "instance/instancer.h"
#include "scene/camera2d.h"
#include "nodes/nodeHandler.h"

int main() {
    // Create a GLFW window
    Window* window = new Window(800, 800, "Example 13: Instance");
    
    // Create a key object for keyboard inputs
    Keyboard* keys = new Keyboard(window);
    // Create a mouse object for mouse input
    Mouse* mouse = new Mouse(window);
    mouse->setGrab();

    // Create a camera object
    Camera camera3d({-3, 0, 0});
    Camera2D camera2d({0, 0});

    // Load shader from file
    Shader* shader3d = new Shader("shaders/entity_3d.vert", "shaders/entity_3d.frag");
    Shader* shader2d = new Shader("shaders/entity_2d.vert", "shaders/entity_2d.frag");
    
    // Data for making node
    Mesh* cube = new Mesh("models/cube.obj");
    Mesh* quad = new Mesh("models/quad.obj");
    Image* image = new Image("textures/container.jpg");
    Texture* texture = new Texture(image);
    texture->setFilter(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
        
    // Create entities
    NodeHandler* nodeHandler = new NodeHandler();

    Node* node3d = new Node(shader3d, cube, texture);
    Node2D* node2d = new Node2D(shader2d, quad, texture, {100, 100});

    nodeHandler->add(node3d);
    nodeHandler->add(node2d);

    // Main loop continues as long as the window is open
    while (window->isRunning()) {
        // Fill the screen with a low blue color
        window->clear(0.2, 0.3, 0.3, 1.0);
        // Update Mouse
        mouse->update();
        if (keys->getPressed(GLFW_KEY_ESCAPE)) {
            mouse->setVisible();
        }
        if (mouse->getClicked()) {
            mouse->setGrab();
        }

        glm::vec2 pos = node2d->getPosition();
        pos.x += keys->getPressed(GLFW_KEY_RIGHT) - keys->getPressed(GLFW_KEY_LEFT);
        pos.y += keys->getPressed(GLFW_KEY_DOWN) - keys->getPressed(GLFW_KEY_UP);
        node2d->setPosition(pos);

        // Update the camera for movement
        camera2d.setPosition({camera3d.getX() * 10, camera3d.getY() * 10});
        camera3d.update(mouse, keys);
        camera2d.update(mouse, keys);
        camera3d.use(shader3d);
        camera2d.use(shader2d);
        
        nodeHandler->render();

        // Show the screen
        window->render();
    }

    // Free memory allocations
    delete image;
    delete texture;
    delete node3d;
    delete node2d;
    delete shader3d;
    delete shader2d;
    delete nodeHandler;
    delete cube;
    delete quad;
    delete window;
    delete keys;
    delete mouse;
}