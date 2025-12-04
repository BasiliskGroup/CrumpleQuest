#include "game/paperView.h"
#include "game/game.h"
#include <iostream>

PaperView::PaperView(Game* game): game(game) {
    engine = game->getEngine();
    backgroundShader = new Shader("shaders/background.vert", "shaders/background.frag");
    scene = new Scene(engine, backgroundShader);
    frame = new Frame(engine, 2400, 900);
    camera = new StaticCamera(engine);

    // Set up shader for paper
    paperShader = new Shader("shaders/default.vert", "shaders/default.frag");
    paperShader->bind("uTexture", frame->getFBO(), 5);
    
    // Set up VAO for paper (will be initialized with empty data, regenerated when paper is created)
    std::vector<float> quadVertices; // Start with empty - will be populated when paper exists
    paperVBO = new VBO(quadVertices);
    paperVAO = new VAO(paperShader, paperVBO);

    // Set up camera
    camera->use(paperShader);
    camera->setX(0.0);
    camera->setZ(0.9);
    camera->setYaw(-90.0);
    camera->setAspect(16.0 / 9.0);
    scene->setCamera(camera);

    // Set up paper model
    paperModel = glm::mat4(1);
    paperShader->setUniform("uModel", paperModel);

    // Set up rotation data
    lastRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    // Set up table
    Node* table = new Node(scene, {.mesh=game->getMesh("quad3D"), .material=game->getMaterial("table"), .position={0.0, 0.0, -1.0}, .scale={5.0, 5.0, 1.0}});
    
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
    frame->clear(0.0, 1.0, 0.0, 1.0);

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
void PaperView::update() {
    
    scene->update();

    Mouse* mouse = engine->getMouse();
    float width = (float)engine->getWindow()->getWidth();
    float height = (float)engine->getWindow()->getHeight();

    if (mouse->getLeftClicked()) {
        mouseStart = glm::vec2(mouse->getX(), mouse->getY());
        mouseStartVector = mapToSphere(mouseStart.x, mouseStart.y, width, height);
    }
    else if (mouse->getLeftDown()) {
        glm::vec2 mouseCurrent = glm::vec2(mouse->getX(), mouse->getY());
        glm::vec3 mouseCurrentVector = mapToSphere(mouseCurrent.x, mouseCurrent.y, width, height);
        
        glm::vec3 rotationAxis = glm::cross(mouseStartVector, mouseCurrentVector);
        float angle = glm::acos(glm::dot(mouseStartVector, mouseCurrentVector));
        
        // Check for small axis vector to prevent crash on normalization
        if (glm::length2(rotationAxis) > 1e-6f) {
            currentRotation = glm::angleAxis(angle, glm::normalize(rotationAxis));
        } else {
            currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity
        }
    }
    else if (mouse->getLeftReleased()) {
        // Accumulate rotation using quaternion multiplication (new * old)
        lastRotation = currentRotation * lastRotation;
        currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Reset temporary drag rotation
    }

    glm::quat combinedRotation = currentRotation * lastRotation;
    glm::mat4 paperModel = glm::toMat4(combinedRotation);

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

