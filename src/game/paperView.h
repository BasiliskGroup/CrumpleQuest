#ifndef PAPER_VIEW_H
#define PAPER_VIEW_H

#include "util/includes.h"
#include "levels/paper.h"

class Game;

class PaperView {
    private:
        Game* game;
        Engine* engine;
        Scene* scene;
        Frame* frame;
        StaticCamera* camera;

        // Custome paper render
        Shader* paperShader;
        Shader* backgroundShader;
        VBO* paperVBO;
        VAO* paperVAO;
        glm::vec3 paperPosition;
        glm::mat4 paperModel;

        // Arcball for paper rotating
        glm::vec2 mouseStart;
        glm::vec3 mouseStartVector;
        glm::quat currentRotation;
        glm::quat lastRotation;
        glm::quat targetRotation;


    public:
        PaperView(Game* game);
        ~PaperView();
        
        void update(Paper* paper);
        void render();
        void renderLevelFBO(Paper* paper);
        void regenerateMesh(); // Regenerate mesh data from current paper
};

#endif