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

        // director nodes
        Node* topSign;
        Node* bottomSign;
        Node* leftSign;
        Node* rightSign;

        std::pair<vec3, vec3> topBounds;
        std::pair<vec3, vec3> bottomBounds;
        std::pair<vec3, vec3> leftBounds;
        std::pair<vec3, vec3> rightBounds;

        // Directional node visibility state
        bool topVisible = false;
        bool bottomVisible = false;
        bool leftVisible = false;
        bool rightVisible = false;

        // minimap
        std::vector<std::vector<Node*>> minimapNodes;
        vec3 minimapCenter={-1.4, -0.75, 0.5};
        float minimapScale=0.03;
        float minimapSpacing=0.01;

    public:
        PaperView(Game* game);
        ~PaperView();
        
        void update(Paper* paper);
        void render();
        void renderLevelFBO(Paper* paper);
        void regenerateMesh(); // Regenerate mesh data from current paper
        
        Scene* getScene() { return scene; } 

        void switchToRoom(Paper* paper, int dx, int dy);
        void hideDirectionalNodes();
        void showDirectionalNodes(bool top, bool bottom, bool left, bool right);

        void createMinimap();
        bool isTransitioning() const { return transitionTimer > 0.0f; }
};

#endif