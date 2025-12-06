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
        int currentTrackIndex;  // Track which music track we're on (0=parchment, 1=notebook, 2=grid)

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
        vec3 minimapCenter={-1.4, -0.9, 0.5};
        float minimapScale=0.03;
        float minimapSpacing=0.01;

        // boss battle
        Node* bossNode;
        glm::vec3 bossBasePosition;  // Base position for boss node (topBounds.second)
        glm::vec3 bossHorizontalDirection;  // Horizontal direction for sliding (planeRight)
        glm::vec3 bossVerticalDirection;  // Vertical direction on paper plane (planeUp)
        glm::vec3 bossPlaneNormal;  // Plane normal (direction from camera to paper)

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
        void clearMinimap();
        void showGameElements(); // Create health tokens and directional nodes
        void hideGameElements(); // Delete health tokens and directional nodes
        bool isTransitioning() const { return transitionTimer > 0.0f; }
        
        // Boss node access
        Node* getBossNode() { return bossNode; }
        glm::vec3 getBossBasePosition() const { return bossBasePosition; }
        glm::vec3 getBossHorizontalDirection() const { return bossHorizontalDirection; }
        glm::vec3 getBossVerticalDirection() const { return bossVerticalDirection; }
        glm::vec3 getBossPlaneNormal() const { return -bossPlaneNormal; }
        glm::vec3 getPaperPosition() const { return paperPosition; }
        
        // Project 3D position onto paper plane and return 2D coordinates relative to paper center
        vec2 projectToPaperPlane(const glm::vec3& worldPos) const;
        
        // Convert 2D coordinates on paper plane to 3D position, with height offset along plane normal
        glm::vec3 paperPlaneTo3D(const vec2& paperPos, float height = 0.0f) const;
        
        // Clamp a 3D position to be at least minDistance above the paper plane along the plane normal
        glm::vec3 clampAbovePaperPlane(const glm::vec3& worldPos, float minDistance = 0.1f) const;
};

#endif