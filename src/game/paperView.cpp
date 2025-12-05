#include "game/paperView.h"
#include "game/game.h"
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
    Node* john = new Node(scene, {.mesh=game->getMesh("john"), .material=game->getMaterial("john"), .position={1.8, -0.75, 1.1}, .rotation=johnRotation, .scale={0.15, 0.15, 0.15}});
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

