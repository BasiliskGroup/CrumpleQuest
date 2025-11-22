#include "levels/levels.h"
#include "ui/ui.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    scene(nullptr),
    voidScene(nullptr),
    camera(nullptr),
    paper(nullptr),
    paperNode(nullptr)
{
    // basilisk preamble
    this->engine = new Engine(800, 800, "Crumple Quest");
    this->scene = new Scene2D(this->engine);
    this->scene->getSolver()->setGravity(0);
    this->camera = new StaticCamera2D(engine);
    this->scene->setCamera(this->camera);
    
    // storing templates
    this->voidScene = new Scene2D(this->engine);
}

Game::~Game() {
    delete player; player = nullptr;
    delete floor; floor = nullptr;

    // materials
    for (auto const& [name, material] : materials) {
        delete material;
    }
    materials.clear();

    // images
    for (auto const& [name, image] : images) {
        delete image;
    }
    images.clear();

    // meshes
    for (auto const& [name, mesh] : meshes) {
        delete mesh;
    }
    meshes.clear();

    for (Enemy* enemy : enemies) {
        delete enemy;
    }
    enemies.clear();

    delete paper; paper = nullptr;

    // scene2D will handle deletion
    uiElements.clear();

    // basilisk closing, must be last
    delete engine; engine = nullptr;
    delete scene; scene = nullptr;
    delete camera; camera = nullptr;
    delete voidScene; voidScene = nullptr;
}

void Game::update(float dt) {
    // always unhide mouse 
    this->engine->getMouse()->setVisible();

    // keyboard
    auto keys = this->engine->getKeyboard();
    if (keys->getPressed(GLFW_KEY_F) && kWasDown == false) {
        paper->flip();
    }
    kWasDown = keys->getPressed(GLFW_KEY_F);

    // folding
    bool leftIsDown = engine->getMouse()->getLeftDown();
    vec2 mousePos = { engine->getMouse()->getX(), engine->getMouse()->getY() };

    std::cout << engine->getMouse()->getWorldX(scene->getCamera()) << " " << engine->getMouse()->getWorldX(scene->getCamera()) << std::endl;

    // hard code mouse to world coordinates TODO replace this with world coordinate mouse call
    mousePos -= vec2{ 400, 400 };
    mousePos /= 80;
    mousePos.y *= -1;

    if (!leftWasDown && leftIsDown) { // we just clicked
        LeftStartDown = mousePos;
        
        if (paper) {
            paper->activateFold(mousePos);
        }

    } else if (leftWasDown && !leftIsDown) { // we just let go
        if (paper) {
            paper->fold(LeftStartDown, mousePos);
            paper->deactivateFold();
        }
    }

    // continuous fold
    // if (leftIsDown && paper) {
    //     paper->fold(LeftStartDown, mousePos);
    // }

    // update buttons
    for (UIElement* elem : uiElements) {
        elem->event(mousePos, leftIsDown);
    }

    leftWasDown = leftIsDown;

    // entity update
    if (player != nullptr) {
        player->move(dt);
    }

    for (Enemy* enemy : enemies) {
        enemy->move(player->getPosition(), dt);
    }

    // add paper stuff to the scene
    if (paper) {
        if (paperNode == nullptr) {
            paperNode = new Node2D(scene, { .mesh=meshes["quad"], .material=materials["test"] });
        }

        // reconstruct meshes TODO bring this back into paper class
        paper->paperMeshes.first->regenerateMesh();
        paper->paperMeshes.second->regenerateMesh();
        
        Mesh* paperMesh = paper->getMesh();
        if (paperMesh != nullptr) paperNode->setMesh(paperMesh);

    } else {
        if (paperNode) {
            delete paperNode;
            paperNode = nullptr;
        }
    }

    // basilisk update
    engine->update();
    scene->update();
    scene->render();
    engine->render();
}
