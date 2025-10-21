#ifndef CAMERA_2D_H
#define CAMERA_2D_H


#include "util/includes.h"
#include "render/shader.h"
#include "IO/mouse.h"
#include "IO/keyboard.h"


class Camera2D {
    private:
        glm::vec2 position;

        glm::mat4 projection;
        glm::mat4 view;
    
        void updateProjection();
        void updateView();

    public:
        Camera2D(glm::vec2 position={0.0f, 0.0f});

        void update(Mouse* mouse, Keyboard* keys);
        void use(Shader* shader);

        void setPosition(glm::vec2 position) { this->position = position; }
};


#endif