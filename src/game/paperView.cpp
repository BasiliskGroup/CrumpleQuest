#include "game/paperView.h"
#include "game/game.h"
#include "levels/floor.h"
#include "audio/sfx_player.h"
#include "audio/music_player.h"
#include <iostream>

PaperView::PaperView(Game* game): game(game) {
    engine = game->getEngine();
    backgroundShader = new Shader("shaders/background.vert", "shaders/background.frag");
    scene = new Scene(engine, backgroundShader);
    frame = new Frame(engine, 2400, 900);
    camera = new StaticCamera(engine, {0, 1.094, 0.1266});

    // Set up shader for paper
    paperShader = new Shader("shaders/default.vert", "shaders/default.frag");
    paperShader->bind("uTexture", frame->getFBO(), 5);
    
    // Set up VAO for paper (will be initialized with empty data, regenerated when paper is created)
    std::vector<float> quadVertices; // Start with empty - will be populated when paper exists
    paperVBO = new VBO(quadVertices);
    paperVAO = new VAO(paperShader, paperVBO);
    paperPosition = glm::vec3(0.0, 0.1386, 0.544);
    fixedPaperPosition = paperPosition;  // Store fixed position for directional nodes (never changes)
    transitionTarget = paperPosition;

    // Set up camera
    camera->use(paperShader);
    glm::vec3 cameraPos = camera->getPosition();

    // Calculate direction from camera to paper
    glm::vec3 direction = glm::normalize(paperPosition - cameraPos);

    // Calculate yaw (rotation around Y axis)
    float yaw = glm::degrees(atan2(direction.x, direction.z));

    // Calculate pitch (rotation around X axis)
    float pitch = glm::degrees(asin(-direction.y));

    camera->setYaw(yaw - 270.0);
    camera->setPitch(-pitch);
    camera->setAspect(16.0 / 9.0);
    scene->setCamera(camera);

    // Set up paper model
    paperModel = glm::mat4(1);
    paperModel = glm::translate(paperModel, paperPosition);
    paperShader->setUniform("uModel", paperModel);

    // Set up rotation data
    lastRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    targetRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Set up table
    Node* table = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("table"), .position={0.0, -1.0, 0.0}, .rotation=glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={5.0, 5.0, 1.0}});
    Node* rug_desaturated = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("rug_desaturated"), .position={0.0, -2.0, 2.0}, .rotation=glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={12.0, 8.0, 1.0}});
    Node* mug = new Node(scene, {.mesh=game->getMesh("mug"), .material=game->getMaterial("lightGrey"), .position={-1.8, -0.75, 1.5}, .rotation=glm::angleAxis(glm::radians(110.0f), glm::vec3(0.0f, 1.0f, 0.0f)), .scale={0.3, 0.3, 0.3}});
    
    glm::quat johnRotation = glm::angleAxis(glm::radians(-13.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    Node* john = new Node(scene, {.mesh=game->getMesh("john"), .material=game->getMaterial("john"), .position={1.8, -0.75, 1.8}, .rotation=johnRotation, .scale={0.15, 0.15, 0.15}});

    // Health token positions (will be created in showGameElements())
    healthActivePositions = {{2.0, -.95, 1.00}, {1.75, -.95, 0.5}, {1.9, -.95, 1.35}, {1.5, -.95, 1.15}, {1.5, -.95, 0.75}, {1.3, -.95, 0.3}};
    // Inactive positions are off-screen to the right (x + 1)
    healthInactivePositions = {{3.0, -.95, 1.00}, {2.75, -.95, 0.5}, {2.9, -.95, 1.35}, {2.5, -.95, 1.15}, {2.5, -.95, 0.75}, {2.3, -.95, 0.3}};

    transitionDuration = 0.4f;
    transitionDistance =  3.5f;
    transitionTimer = 0.0f;
    transitionState = 0;
    currentTrackIndex = 2; // Start on grid track (0=parchment, 1=notebook, 2=grid)

    // Directional nodes will be created in showGameElements()
    topSign = nullptr;
    bottomSign = nullptr;
    rightSign = nullptr;
    leftSign = nullptr;
    bossNode = nullptr;

    // Crosshair
    crosshairScene = new Scene2D(engine);
    crosshair = new Node2D(crosshairScene, {.mesh=game->getMesh("quad"), .material=game->getMaterial("crosshair"), .scale={0.5f, 0.5f}});
}

PaperView::~PaperView() {
    delete paperVBO;
    delete paperVAO;
    delete paperShader;
    delete camera;
    delete frame;
    delete crosshairScene;
    delete scene;
}

/**
 * @brief Render the level onto the paper frame. This does not render to the screen
 * 
 */
void PaperView::renderLevelFBO(Paper* paper) {
    frame->use();
    frame->clear(0.0, 0.0, 0.0, 0.0);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    glViewport(0, 0, 1200, 900);
    paper->getFirstSide()->getScene()->render();
    glViewport(1200, 0, 1200, 900);
    paper->getSecondSide()->getScene()->render();

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

/**
 * @brief Render the paper and 3D scene to the currently bound render target
 * 
 */
void PaperView::render() {
    // Health token transitions (animate hearts even when no game is active)
    if (!healthTokens.empty()) {
        for (int i = 0; i < 6; i++) {
            Node* token = healthTokens.at(i);
            glm::vec3 targetPosition = healthActivePositions.at(i);
            // If player exists and doesn't have this health, move token off-screen
            if (game->getPlayer() && (6 - i) > game->getPlayer()->getHealth()) {
                targetPosition.x += 1;
            }

            glm::vec3 moveVector = targetPosition - token->getPosition();
            token->setPosition((token->getPosition() + 5.0f * moveVector * (float)engine->getDeltaTime()));
        }
    }
    
    // Update scene (needed for heart animations and other node updates)
    scene->update();
    
    // Render background 3d objects
    scene->render();
    camera->use(paperShader);
    paperShader->use();
    paperVAO->render();

    crosshairScene->update();
    glDisable(GL_DEPTH_TEST);
    crosshairScene->render();
    glEnable(GL_DEPTH_TEST);
}

glm::vec3 mapToSphere(float x, float y, float width, float height) {
    // 1. Normalize coordinates to [-1, 1] range
    float x_ndc = (2.0f * x / width) - 1.0f;
    // Note: The Y-axis is flipped here because screen Y is often top-down
    float y_ndc = 1.0f - (2.0f * y / height); 

    glm::vec3 v = glm::vec3(x_ndc, y_ndc, 0.0f);
    float r2 = x_ndc * x_ndc + y_ndc * y_ndc;

    if (r2 <= 1.0f) {
        // Point is inside the disk, calculate Z based on unit sphere equation
        v.z = glm::sqrt(1.0f - r2);
    } else {
        // Point is outside the disk, project it to the boundary
        // and normalize it to unit length (r2 > 1, so length > 1)
        v = glm::normalize(v);
        v.z = 0.0f;
    }
    return v;
}

/**
 * @brief 
 * 
 */
void PaperView::update(Paper* paper) {

    crosshair->setPosition({game->getEngine()->getMouse()->getWorldX(crosshairScene->getCamera()), game->getEngine()->getMouse()->getWorldY(crosshairScene->getCamera())});
    
    // Paper transtions
    if (transitionTimer > 0.0f) {
        // Play outgoing paper sound at the start of transition
        if (transitionTimer >= transitionDuration - engine->getDeltaTime()) {
            audio::SFXPlayer::Get().Play("slide");
        }
        
        transitionTimer -= engine->getDeltaTime();
        if (transitionTimer < 0.0f) {
            std::cout << "[PaperView] Transition complete: dx=" << transitionDirection.x << ", dy=" << transitionDirection.y << std::endl;
            // Just update visual position - room switch already happened
            paperPosition = glm::vec3(0.0 - transitionDirection.x * transitionDistance, -1.0, 0.544 - transitionDirection.y * transitionDistance);
            transitionTarget = glm::vec3(0.0, 0.1386, 0.544);
            // Play incoming paper sound when new paper starts moving in
            audio::SFXPlayer::Get().Play("pickup");
            
            // Cycle to next music track (for testing)
            currentTrackIndex = (currentTrackIndex + 1) % 3;
            const char* trackNames[] = {"parchment", "notebook", "grid"};
            audio::MusicPlayer::Get().FadeTo(trackNames[currentTrackIndex], 2.0f);
            
            this->game->switchToRoom(nextPaper, transitionDirection.x, transitionDirection.y);
            this->regenerateMesh();

        }        
    }

    glm::vec3 moveVector = paperPosition - transitionTarget;
    paperPosition = paperPosition - 3.0f * moveVector * (float)engine->getDeltaTime();

    Mouse* mouse = engine->getMouse();
    float width = (float)engine->getWindow()->getWidth() * engine->getWindow()->getWindowScaleX();
    float height = (float)engine->getWindow()->getHeight() * engine->getWindow()->getWindowScaleY();
    float deltaTime = engine->getDeltaTime();
    elapsedTime += deltaTime;

    glm::vec3 cameraPos = camera->getPosition();

    // Calculate direction from paper to camera
    glm::vec3 direction = glm::normalize(cameraPos - paperPosition);

    // Define up vector
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    // Create a rotation matrix that looks at the camera while staying upright
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    glm::vec3 newUp = glm::cross(direction, right);

    // Build rotation matrix from the orthonormal basis
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = right;
    rotationMatrix[1] = newUp;
    rotationMatrix[2] = direction;

    glm::quat defaultPosition = glm::quat_cast(rotationMatrix);

    if (paper->getCurrentSide()) {
        // Rotate 180 degrees around the UP axis to spin around and show back side
        glm::quat flip = glm::angleAxis(glm::radians(180.0f), newUp);
        defaultPosition = flip * defaultPosition;
    }

    if (mouse->getMiddleClicked()) {
        mouseStart = glm::vec2(mouse->getX(), mouse->getY());
        lastMousePos = mouseStart;
        mouseStartVector = mapToSphere(mouseStart.x, mouseStart.y, width, height);
        // Bake current rotation into lastRotation when starting a new drag
        lastRotation = currentRotation;

        // Play sfx
        audio::SFXPlayer::Get().Play("rotate");
    }
    else if (mouse->getMiddleDown()) {
        glm::vec2 mouseCurrent = glm::vec2(mouse->getX(), mouse->getY());
        glm::vec3 mouseCurrentVector = mapToSphere(mouseCurrent.x, mouseCurrent.y, width, height);
        
        // Calculate mouse velocity for sound effects
        float mouseVelocity = glm::length(mouseCurrent - lastMousePos) / deltaTime;
        float velocityThreshold = 1500.0f; // Pixels per second
        float soundCooldown = 0.15f; // Minimum time between sounds
        
        if (mouseVelocity > velocityThreshold && (elapsedTime - lastSoundTime) > soundCooldown) {
            // Scale volume based on velocity (from threshold to 2x threshold = 0.15 to 0.65)
            float normalizedVelocity = (mouseVelocity - velocityThreshold) / velocityThreshold;
            float volume = glm::clamp(0.15f + normalizedVelocity * 0.5f, 0.15f, 0.65f);
            audio::SFXPlayer::Get().PlayWithVolume("fold", volume);
            lastSoundTime = elapsedTime;
        }
        
        lastMousePos = mouseCurrent;
        
        glm::vec3 rotationAxis = glm::cross(mouseStartVector, mouseCurrentVector);
        float angle = glm::acos(glm::dot(mouseStartVector, mouseCurrentVector));

        float sensitivity = 2.5f;
        angle *= sensitivity;
        
        // Check for small axis vector to prevent crash on normalization
        if (glm::length2(rotationAxis) > 1e-6f) {
            // Transform rotation axis from screen space to world space
            // Use the camera's orientation basis vectors
            glm::vec3 worldRotationAxis = rotationAxis.x * right + 
                                        rotationAxis.y * newUp + 
                                        rotationAxis.z * direction;
            
            targetRotation = glm::angleAxis(angle, glm::normalize(worldRotationAxis)) * lastRotation;
        } else {
            targetRotation = lastRotation;
        }

        // Play sfx if fast movement
    }
    else if (mouse->getMiddleReleased()) {
        // Set target back to default
        targetRotation = defaultPosition;
        lastRotation = defaultPosition;

        // Play sfx
        audio::SFXPlayer::Get().Play("rotate");
    }
    else {
        targetRotation = defaultPosition;
    }

    // Smooth interpolation using slerp (spherical linear interpolation)
    float smoothingFactor = 5.0f;
    float t = glm::clamp(smoothingFactor * deltaTime, 0.0f, 1.0f);
    currentRotation = glm::slerp(currentRotation, targetRotation, t);

    paperModel = glm::translate(glm::mat4(1), paperPosition) * glm::toMat4(currentRotation);
    paperShader->setUniform("uModel", paperModel);

    // Smooth interpolation for directional nodes
    float nodeSmoothingFactor = 5.0f;
    float nodeT = glm::clamp(nodeSmoothingFactor * deltaTime, 0.0f, 1.0f);
    
    // Interpolate each directional node to its target position
    if (topSign) {
        glm::vec3 targetPos = topVisible ? topBounds.second : topBounds.first;
        glm::vec3 currentPos = topSign->getPosition();
        topSign->setPosition(glm::mix(currentPos, targetPos, nodeT));
    }
    
    if (bottomSign) {
        glm::vec3 targetPos = bottomVisible ? bottomBounds.second : bottomBounds.first;
        glm::vec3 currentPos = bottomSign->getPosition();
        bottomSign->setPosition(glm::mix(currentPos, targetPos, nodeT));
    }
    
    if (rightSign) {
        glm::vec3 targetPos = rightVisible ? rightBounds.second : rightBounds.first;
        glm::vec3 currentPos = rightSign->getPosition();
        rightSign->setPosition(glm::mix(currentPos, targetPos, nodeT));
    }
    
    if (leftSign) {
        glm::vec3 targetPos = leftVisible ? leftBounds.second : leftBounds.first;
        glm::vec3 currentPos = leftSign->getPosition();
        leftSign->setPosition(glm::mix(currentPos, targetPos, nodeT));
    }
    
}

void PaperView::regenerateMesh() {
    Paper* paper = game->getPaper();
    if (paper == nullptr) {
        // Paper doesn't exist yet, use empty mesh
        std::vector<float> emptyVertices;
        delete paperVBO;
        delete paperVAO;
        paperVBO = new VBO(emptyVertices);
        paperVAO = new VAO(paperShader, paperVBO);
        return;
    }
    
    // Generate mesh data from paper
    std::vector<float> quadVertices;
    paper->toData(quadVertices);
    
    // Replace VBO and VAO with new data
    delete paperVBO;
    delete paperVAO;
    paperVBO = new VBO(quadVertices);
    paperVAO = new VAO(paperShader, paperVBO);
}

void PaperView::switchToRoom(Paper* paper, int dx, int dy) {
    std::cout << "[PaperView] switchToRoom called: dx=" << dx << ", dy=" << dy << std::endl;
    
    // Store transition info but don't switch room yet - wait for transition
    transitionDirection = {dx, dy};
    transitionTimer = transitionDuration;
    transitionTarget = glm::vec3(0.0 + dx * transitionDistance, -1.0, 0.544 + dy * transitionDistance);
    nextPaper = paper;
}

void PaperView::hideDirectionalNodes() {
    topVisible = false;
    bottomVisible = false;
    leftVisible = false;
    rightVisible = false;
}

void PaperView::showDirectionalNodes(bool top, bool bottom, bool left, bool right) {
    topVisible = top;
    bottomVisible = bottom;
    leftVisible = left;
    rightVisible = right;
}

void PaperView::createMinimap() {
    // Clear and delete all previous minimap nodes
    for (auto& row : minimapNodes) {
        for (Node* node : row) {
            if (node) {
                delete node;
            }
        }
    }
    minimapNodes.clear();

    minimapCenter = {-1.5, -0.9, 0.6};
    minimapScale = 0.04;
    minimapSpacing = 0.015;
    
    // Get the floor from game
    Floor* floor = game->getFloor();
    if (!floor) {
        return; // No floor available
    }
    
    // Resize minimapNodes to match floor dimensions
    minimapNodes.resize(FLOOR_WIDTH);
    for (auto& row : minimapNodes) {
        row.resize(FLOOR_WIDTH, nullptr);
    }
    
    // Calculate spacing between cubes
    float cubeSize = minimapScale;
    float totalSpacing = 2 * cubeSize + minimapSpacing;
    
    // Calculate offset to center the minimap (since floor is 7x7, center is at 3,3)
    int centerX = FLOOR_WIDTH / 2;
    int centerY = FLOOR_WIDTH / 2;
    
    // Get current player position
    int currentX = floor->getCurrentX();
    int currentY = floor->getCurrentY();
    std::cout << "[Minimap] Creating minimap - Player at (" << currentX << ", " << currentY << ")" << std::endl;
    
    // Create cubes for each room in the playMap
    for (int x = 0; x < FLOOR_WIDTH; x++) {
        for (int y = 0; y < FLOOR_WIDTH; y++) {
            RoomTypes roomType = floor->getRoomType(x, y);
            
            // Only create a cube if there's a room (not NULL_ROOM)
            if (roomType != NULL_ROOM) {
                // Calculate position relative to minimap center
                float offsetX = (x - centerX) * totalSpacing;
                float offsetZ = (y - centerY) * totalSpacing;
                
                glm::vec3 offset = glm::vec3(-offsetX, 0.0f, -offsetZ);
                
                // Rotate the offset around Y-axis by 10 degrees
                glm::quat rotation = glm::angleAxis(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::vec3 rotatedOffset = rotation * offset;
                
                glm::vec3 position = minimapCenter + rotatedOffset;
                // position.x += uniform(-0.005, 0.005);
                // position.z += uniform(-0.005, 0.005);
                
                // Determine material based on room type and position
                Material* material = game->getMaterial("lightGrey"); // Default
                
                // Check if this is the current player room
                if (x == currentX && y == currentY) {
                    material = game->getMaterial("blue");
                }
                // Check if this is the boss room
                else if (roomType == BOSS_ROOM) {
                    material = game->getMaterial("darkred");
                }
                // Check if this is the treasure room
                else if (roomType == TREASURE_ROOM) {
                    material = game->getMaterial("yellow");
                }
                
                // Create cube node with appropriate material
                Node* cube = new Node(scene, {
                    .mesh = game->getMesh("cube"),
                    .material = material,
                    .position = position,
                    .rotation = rotation,  // glm::angleAxis(glm::radians(uniform(-10.0f, 10.0f)), glm::vec3(0.0f, 1.0f, 0.0f)) * 
                    .scale = {minimapScale, minimapScale, minimapScale}
                });
                
                minimapNodes[x][y] = cube;
            }
        }
    }
}

void PaperView::clearMinimap() {
    // Clear and delete all minimap nodes
    for (auto& row : minimapNodes) {
        for (Node* node : row) {
            if (node) {
                delete node;
            }
        }
    }
    minimapNodes.clear();
}

void PaperView::showGameElements() {
    // Create health tokens at inactive positions (off-screen) so they animate in
    for (glm::vec3& pos : healthInactivePositions) {
        glm::quat rotation = glm::angleAxis(glm::radians(90.0f + uniform(-25.0, 25.0)), glm::vec3(0.0f, 1.0f, 0.0f));
        Node* token = new Node(scene, {.mesh=game->getMesh("heart"), .material=game->getMaterial("darkred"), .position=pos, .rotation=rotation, .scale={.08, .08, .08}});
        healthTokens.push_back(token);
    }
    
    // Calculate plane vectors (needed for both directional nodes and boss node)
    // Use fixedPaperPosition for directional nodes so they stay in fixed world space
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(fixedPaperPosition - cameraPos);
    
    glm::vec3 planeNormal = direction;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 planeRight = glm::cross(worldUp, planeNormal);
    planeRight = glm::normalize(planeRight);
    glm::vec3 planeUp = glm::normalize(glm::cross(planeNormal, planeRight));
    float offsetAbove = 0.01f;
    glm::vec3 offsetVector = -planeNormal * offsetAbove;
    float quadDistance = 0.5f;
    
    glm::mat3 planeOrientation = glm::mat3(planeRight, planeUp, planeNormal);
    glm::quat basePlaneRotation = glm::quat_cast(planeOrientation);
    
    // Create directional nodes - only calculate bounds once when nodes are first created
    // Bounds are fixed in world space using fixedPaperPosition (never changes)
    if (!topSign || !bottomSign || !rightSign || !leftSign) {
        // Calculate bounds relative to fixed paper position (fixed in world space)
        topBounds = {fixedPaperPosition + 2.0f * planeUp * quadDistance + offsetVector, fixedPaperPosition + planeUp * quadDistance + offsetVector + planeUp * 0.1f};
        bottomBounds = {fixedPaperPosition - 2.0f * planeUp * quadDistance + offsetVector, fixedPaperPosition - planeUp * quadDistance + offsetVector};
        rightBounds = {fixedPaperPosition + 3.0f * planeRight * quadDistance + offsetVector, fixedPaperPosition + 1.8f * planeRight * quadDistance + offsetVector + planeUp * 0.1f};
        leftBounds = {fixedPaperPosition - 3.0f * planeRight * quadDistance + offsetVector, fixedPaperPosition - 1.8f * planeRight * quadDistance + offsetVector + planeUp * 0.1f};
        
        // Create nodes if they don't exist
        if (!topSign) {
            topSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("hand_up"), .position=topBounds.first, .rotation=(glm::angleAxis(glm::radians(90.0f), planeNormal) * basePlaneRotation), .scale={0.8, 0.6, 0.1}});
        }
        if (!bottomSign) {
            bottomSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("hand_down"), .position=bottomBounds.first, .rotation=(glm::angleAxis(glm::radians(-90.0f), planeNormal) * basePlaneRotation), .scale={0.8, 0.6, 0.1}});
        }
        if (!rightSign) {
            rightSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("hand_right"), .position=rightBounds.first, .rotation=(glm::angleAxis(glm::radians(0.0f), planeNormal) * basePlaneRotation), .scale={0.8, 0.6, 0.1}});
        }
        if (!leftSign) {
            leftSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("hand_left"), .position=leftBounds.first, .rotation=(glm::angleAxis(glm::radians(180.0f), planeNormal) * basePlaneRotation), .scale={0.8, 0.6, 0.1}});
        }
    }
    // If nodes already exist, don't recalculate bounds or update positions
    // They will smoothly slide between their fixed positions based on visibility flags in update()
    
    // Create boss node - position it above the paper but closer than topBounds
    // Use a position between paper and topBounds, closer to paper
    glm::vec3 bossInitialPos = paperPosition + 0.8f * planeUp * quadDistance + offsetVector;
    // Keep z position same as paper (don't move in z direction)
    bossInitialPos.z = paperPosition.z;
    
    // Check if boss is currently active (node exists and is visible/not empty material)
    bool bossActive = (bossNode != nullptr && bossNode->getMaterial() != game->getMaterial("empty"));
    
    // Only create if it doesn't already exist (to avoid leaks if called multiple times)
    if (bossNode == nullptr) {
        bossNode = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("empty"), .position=bossInitialPos, .rotation=(glm::angleAxis(glm::radians(90.0f), planeNormal) * basePlaneRotation), .scale={1.2, 0.72, 0.24}});
        // Set boss-related data when node is first created
        bossBasePosition = bossInitialPos;  // Store base position (above paper, closer than top)
        bossHorizontalDirection = planeRight;  // Store horizontal direction for sliding
        bossVerticalDirection = planeUp;  // Store vertical direction on paper plane (cross of planeNormal and planeRight)
        bossPlaneNormal = planeNormal;  // Store plane normal (direction from camera to paper)
    } else if (!bossActive) {
        // Boss node exists but boss is not active - safe to update rotation/scale/hide
        // Don't reset position - let Boss class manage its own position
        bossNode->setRotation(glm::angleAxis(glm::radians(90.0f), planeNormal) * basePlaneRotation);
        bossNode->setScale(glm::vec3(1.2f, 0.72f, 0.24f));
        // Hide boss node by default (will be shown when boss is created in boss room)
        bossNode->setMaterial(game->getMaterial("empty"));
        // Update boss-related data when boss is not active
        bossBasePosition = bossInitialPos;  // Store base position (above paper, closer than top)
        bossHorizontalDirection = planeRight;  // Store horizontal direction for sliding
        bossVerticalDirection = planeUp;  // Store vertical direction on paper plane (cross of planeNormal and planeRight)
        bossPlaneNormal = planeNormal;  // Store plane normal (direction from camera to paper)
    }
    // If boss is active, don't touch anything - let Boss class manage everything
}

vec2 PaperView::projectToPaperPlane(const glm::vec3& worldPos) const {
    // Calculate paper plane basis vectors (same as in showGameElements)
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(paperPosition - cameraPos);
    
    glm::vec3 planeNormal = direction;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 planeRight = glm::cross(worldUp, planeNormal);
    planeRight = glm::normalize(planeRight);
    glm::vec3 planeUp = glm::normalize(glm::cross(planeNormal, planeRight));
    
    // Get position relative to paper center
    glm::vec3 relativePos = worldPos - paperPosition;
    
    // Project onto the plane by removing component along the normal
    float normalComponent = glm::dot(relativePos, planeNormal);
    glm::vec3 projected = relativePos - normalComponent * planeNormal;
    
    // Express in terms of planeRight and planeUp (2D coordinates relative to paper center)
    // Negate x to fix horizontal flip
    float x = -glm::dot(projected, planeRight);
    float y = glm::dot(projected, planeUp);
    
    return vec2(x, y);
}

glm::vec3 PaperView::paperPlaneTo3D(const vec2& paperPos, float height) const {
    // Calculate paper plane basis vectors (same as in projectToPaperPlane)
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(paperPosition - cameraPos);
    
    glm::vec3 planeNormal = direction;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 planeRight = glm::cross(worldUp, planeNormal);
    planeRight = glm::normalize(planeRight);
    glm::vec3 planeUp = glm::normalize(glm::cross(planeNormal, planeRight));
    
    // Convert 2D coordinates to 3D position on the paper plane
    // Note: x was negated in projectToPaperPlane, so we negate it here too
    glm::vec3 positionOnPlane = paperPosition + (-paperPos.x) * planeRight + paperPos.y * planeUp;
    
    // Add height offset along plane normal (positive height moves away from camera, toward paper)
    glm::vec3 finalPosition = positionOnPlane - planeNormal * height;
    
    return finalPosition;
}

glm::vec3 PaperView::clampAbovePaperPlane(const glm::vec3& worldPos, float minDistance) const {
    // Calculate paper plane basis vectors (same as in projectToPaperPlane)
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(paperPosition - cameraPos);
    
    glm::vec3 planeNormal = direction;
    
    // Calculate distance along plane normal from paper position
    glm::vec3 relativePos = worldPos - paperPosition;
    float distanceAlongNormal = glm::dot(relativePos, -planeNormal);
    
    // If below minimum distance, adjust along plane normal
    if (distanceAlongNormal < minDistance) {
        float adjustment = minDistance - distanceAlongNormal;
        return worldPos - planeNormal * adjustment;
    }
    
    return worldPos;
}

void PaperView::hideGameElements() {
    // Delete health tokens
    for (Node* token : healthTokens) {
        if (token) {
            delete token;
        }
    }
    healthTokens.clear();
    
    // Delete directional nodes (only called at game end)
    if (topSign) {
        delete topSign;
        topSign = nullptr;
    }
    if (bottomSign) {
        delete bottomSign;
        bottomSign = nullptr;
    }
    if (rightSign) {
        delete rightSign;
        rightSign = nullptr;
    }
    if (leftSign) {
        delete leftSign;
        leftSign = nullptr;
    }
    
    // Delete boss node
    if (bossNode) {
        delete bossNode;
        bossNode = nullptr;
    }
    
    // Clear minimap
    clearMinimap();
}