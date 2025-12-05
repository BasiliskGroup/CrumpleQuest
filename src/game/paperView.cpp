#include "game/paperView.h"
#include "game/game.h"
#include "levels/floor.h"
#include "audio/sfx_player.h"
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
    Node* rug = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("rug"), .position={0.0, -2.0, 2.0}, .rotation=glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={12.0, 8.0, 1.0}});
    Node* mug = new Node(scene, {.mesh=game->getMesh("mug"), .material=game->getMaterial("lightGrey"), .position={-1.8, -0.75, 1.5}, .rotation=glm::angleAxis(glm::radians(110.0f), glm::vec3(0.0f, 1.0f, 0.0f)), .scale={0.3, 0.3, 0.3}});
    
    glm::quat johnRotation = glm::angleAxis(glm::radians(-13.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    Node* john = new Node(scene, {.mesh=game->getMesh("john"), .material=game->getMaterial("john"), .position={1.8, -0.75, 1.8}, .rotation=johnRotation, .scale={0.15, 0.15, 0.15}});

    // Health tokens
    healthActivePositions = {{2.0, -.95, 1.00}, {1.75, -.95, 0.5}, {1.9, -.95, 1.35}, {1.5, -.95, 1.15}, {1.5, -.95, 0.75}, {1.3, -.95, 0.3}};

    for (glm::vec3& pos : healthActivePositions) {
        glm::quat rotation = glm::angleAxis(glm::radians(90.0f + uniform(-25.0, 25.0)), glm::vec3(0.0f, 1.0f, 0.0f));
        Node* token = new Node(scene, {.mesh=game->getMesh("heart"), .material=game->getMaterial("darkred"), .position=pos, .rotation=rotation, .scale={.08, .08, .08}});
        healthTokens.push_back(token);
    }

    transitionDuration = 0.4f;
    transitionDistance =  3.5f;
    transitionTimer = 0.0f;
    transitionState = 0; 

    // create directional nodes
    glm::vec3 planeNormal = direction;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 planeRight = glm::cross(worldUp, planeNormal); // NOTE not safe if planeNormal is parallel to worldUp
    planeRight = glm::normalize(planeRight);
    glm::vec3 planeUp = glm::normalize(glm::cross(planeNormal, planeRight));
    float offsetAbove = 0.01f;
    glm::vec3 offsetVector = -planeNormal * offsetAbove;
    float quadDistance = 0.5f;

    topBounds = {paperPosition + 2.0f * planeUp * quadDistance + offsetVector, paperPosition + planeUp * quadDistance + offsetVector};
    bottomBounds = {paperPosition - 2.0f * planeUp * quadDistance + offsetVector, paperPosition - planeUp * quadDistance + offsetVector};
    rightBounds = {paperPosition + 4.0f * planeRight * quadDistance + offsetVector, paperPosition + 2.0f * planeRight * quadDistance + offsetVector};
    leftBounds = {paperPosition - 4.0f * planeRight * quadDistance + offsetVector, paperPosition - 2.0f * planeRight * quadDistance + offsetVector};
    
    // create directional nodes positioned on the plane
    topSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("blue"), .position=topBounds.first, .rotation=glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={0.1, 0.1, 0.1}});
    bottomSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("blue"), .position=bottomBounds.first, .rotation=glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={0.1, 0.1, 0.1}});
    rightSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("blue"), .position=rightBounds.first, .rotation=glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={0.1, 0.1, 0.1}});
    leftSign = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("blue"), .position=leftBounds.first, .rotation=glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)), .scale={0.1, 0.1, 0.1}});
}

PaperView::~PaperView() {
    delete paperVBO;
    delete paperVAO;
    delete paperShader;
    delete camera;
    delete frame;
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
    // Render background 3d objects
    scene->render();
    camera->use(paperShader);
    paperShader->use();
    paperVAO->render();
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

    // Paper transtions
    if (transitionTimer > 0.0f) {
        transitionTimer -= engine->getDeltaTime();
        if (transitionTimer < 0.0f) {
            std::cout << "[PaperView] Transition complete: dx=" << transitionDirection.x << ", dy=" << transitionDirection.y << std::endl;
            // Just update visual position - room switch already happened
            paperPosition = glm::vec3(0.0 - transitionDirection.x * transitionDistance, -1.0, 0.544 - transitionDirection.y * transitionDistance);
            transitionTarget = glm::vec3(0.0, 0.1386, 0.544);
            // Don't call switchToRoom again - it was already called immediately
        }        
    }

    glm::vec3 moveVector = paperPosition - transitionTarget;
    paperPosition = paperPosition - 3.0f * moveVector * (float)engine->getDeltaTime();

    // Health token transitions
    for (int i = 0; i < 6; i++) {
        Node* token = healthTokens.at(i);
        glm::vec3 targetPosition = healthActivePositions.at(i);
        if ((6 - i) > game->getPlayer()->getHealth()) { // Does not have this token
            targetPosition.x += 1;
        }

        moveVector = token->getPosition() - targetPosition;
        token->setPosition((token->getPosition() - 5.0f * moveVector * (float)engine->getDeltaTime()));
    }

    scene->update();

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
    
    // Switch room immediately for gameplay
    this->game->switchToRoom(paper, dx, dy);
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
                
                glm::vec3 offset = glm::vec3(offsetX, 0.0f, offsetZ);
                
                // Rotate the offset around Y-axis by 10 degrees
                glm::quat rotation = glm::angleAxis(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::vec3 rotatedOffset = rotation * offset;
                
                glm::vec3 position = minimapCenter + rotatedOffset;
                
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
                
                // Create cube node with appropriate material
                Node* cube = new Node(scene, {
                    .mesh = game->getMesh("cube"),
                    .material = material,
                    .position = position,
                    .rotation = rotation,
                    .scale = {minimapScale, minimapScale, minimapScale}
                });
                
                minimapNodes[x][y] = cube;
            }
        }
    }
}