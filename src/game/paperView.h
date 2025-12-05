#ifndef PAPER_VIEW_H
#define PAPER_VIEW_H

#include "util/includes.h"
#include "util/random.h"
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
        
        // For velocity-based rotation sounds
        glm::vec2 lastMousePos;
        float lastSoundTime = 0.0f;
        float elapsedTime = 0.0f;

        // Level transition
        Paper* nextPaper;
        glm::vec2 transitionDirection;
        int  transitionState;
        float transitionDuration;
        float transitionDistance;
        float transitionTimer;
        glm::vec3 transitionTarget;

        // Health tokens
        std::vector<glm::vec3> healthActivePositions;
        std::vector<glm::vec3> healthInactivePositions;
        std::vector<Node*> healthTokens;


    public:
        PaperView(Game* game);
        ~PaperView();
        
        void update(Paper* paper);
        void render();
        void renderLevelFBO(Paper* paper);
        void regenerateMesh(); // Regenerate mesh data from current paper
        
        Scene* getScene() { return scene; } 

        void switchToRoom(Paper* paper, int dx, int dy);
};

#endif