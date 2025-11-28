#include "levels/levels.h"
#include "ui/ui.h"

Game::Game() : 
    player(nullptr), 
    floor(nullptr),
    engine(nullptr),
    currentSide(nullptr),
    paper(nullptr),
    paperNode(nullptr)
{
    this->engine = new Engine(800, 450, "Crumple Quest");
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

    delete paper; paper = nullptr;
    currentSide = nullptr;

    // scene2D will handle deletion
    uiElements.clear();

    // basilisk closing, must be last
    delete engine; engine = nullptr;
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
    vec2 mousePos = { engine->getMouse()->getWorldX(getScene()->getCamera()), engine->getMouse()->getWorldY(getScene()->getCamera()) };

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

    // add paper stuff to the scene
    if (paper) {
        if (paperNode == nullptr) {
            paperNode = new Node2D(getScene(), { .mesh=meshes["quad"], .material=materials["test"] });
            paperNode->setLayer(-0.8);
        }
        
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
    currentSide->update(player->getPosition(), dt);
    engine->render();
}

void Game::setPaper(std::string str) { 
    this->paper = Paper::templates[str]();
    this->currentSide = this->paper->getSingleSide();
    this->paper->setGame(this);
}