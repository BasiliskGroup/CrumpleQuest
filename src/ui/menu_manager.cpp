#include "ui/menu_manager.h"
#include "ui/menu.h"
#include "ui/button.h"
#include "ui/slider.h"
#include "game/game.h"
#include <iostream>

MenuManager::MenuManager(Game* game) : game(game) {
    menuStack = new MenuStack();
}

MenuManager::~MenuManager() {
    delete menuStack;
}

void MenuManager::pushMainMenu() {
    Menu* mainMenu = createMainMenu();
    menuStack->push(mainMenu);
}

void MenuManager::pushSettingsMenu() {
    Menu* settingsMenu = createSettingsMenu();
    menuStack->push(settingsMenu);
}

void MenuManager::popMenu() {
    if (!menuStack || !menuStack->top()) {
        return;
    }
    Menu* menu = menuStack->top();
    menuStack->pop();
    delete menu;  // Delete the menu when hiding it
}

void MenuManager::handleEvent(const vec2& mousePos, bool mouseDown) {
    menuStack->handleEvent(mousePos, mouseDown);
}

void MenuManager::update(float dt) {
    // Delete any menus marked for deletion
    for (Menu* menu : pendingDelete) {
        delete menu;
    }
    pendingDelete.clear();
    
    menuStack->update(dt);
}

Menu* MenuManager::createMainMenu() {
    Menu* mainMenu = new Menu(game);
    
    // Main menu at layer 0.1-0.3
    // Create background
    Node2D* background = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("paper"),
        .scale = {16, 9}
    });
    background->setLayer(0.1f);
    mainMenu->addNode(background);
    
    // Create title
    Node2D* title = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("paper"),
        .position = {0, 3},
        .scale = {6, 2}
    });
    title->setLayer(0.2f);
    mainMenu->addNode(title);

    // Button layout
    float buttonY = 0.5f;
    float buttonSpacing = 3.0f;
    float buttonWidth = 2.0f;
    float buttonHeight = 1.0f;

    // Settings button (left)
    Button* settingsButton = new Button(game->getMenuScene(), game, 
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {-buttonSpacing, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() { 
                this->pushSettingsMenu();
            }
        }
    );
    settingsButton->setLayer(0.3f);
    mainMenu->addElement(settingsButton);

    // Start button (center)
    Button* startButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("lightGrey"),
            .position = {0, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() {
                // Mark menu for deletion after this callback completes
                Menu* menu = this->menuStack->top();
                this->menuStack->clear();
                this->pendingDelete.push_back(menu);
                this->game->startGame();
            }
        }
    );
    startButton->setLayer(0.3f);
    mainMenu->addElement(startButton);

    // Quit button (right)
    Button* quitButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("man"),
            .position = {buttonSpacing, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() { 
                glfwSetWindowShouldClose(this->game->getEngine()->getWindow()->getWindow(), GLFW_TRUE);
            }
        }
    );
    quitButton->setLayer(0.3f);
    mainMenu->addElement(quitButton);
    
    return mainMenu;
}

Menu* MenuManager::createSettingsMenu() {
    Menu* settingsMenu = new Menu(game);
    
    // Create background
    Node2D* background = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("box"),
        .scale = {16, 9}
    });
    background->setLayer(0.4f);
    settingsMenu->addNode(background);
    
    // Create title
    Node2D* title = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("lightGrey"),
        .position = {0, 3},
        .scale = {4, 1.5}
    });
    title->setLayer(0.5f);
    settingsMenu->addNode(title);

    // Sliders layout
    float sliderStartY = 1.5f;
    float sliderSpacing = 1.2f;
    float iconX = -3.5f;
    float sliderMinX = -2.5f;
    float sliderMaxX = 2.5f;
    float iconSize = 0.4f;
    
    // Master volume slider
    Node2D* masterIcon = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("knight"),
        .position = {iconX, sliderStartY},
        .scale = {iconSize, iconSize}
    });
    masterIcon->setLayer(0.55f);
    settingsMenu->addNode(masterIcon);
    
    Slider* masterSlider = new Slider(game->getMenuScene(), game, 
        {sliderMinX, sliderStartY}, 
        {sliderMaxX, sliderStartY}, 
        {.pegMaterial = game->getMaterial("box")}
    );
    masterSlider->getBar()->setLayer(0.54f);
    masterSlider->getPeg()->setLayer(0.56f);
    masterSlider->setCallback([this](float proportion) {
        // TODO: Set master volume
        std::cout << "Master volume: " << proportion << std::endl;
    });
    settingsMenu->addElement(masterSlider);
    
    // Music volume slider
    Node2D* musicIcon = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("sword"),
        .position = {iconX, sliderStartY - sliderSpacing},
        .scale = {iconSize, iconSize}
    });
    musicIcon->setLayer(0.55f);
    settingsMenu->addNode(musicIcon);
    
    Slider* musicSlider = new Slider(game->getMenuScene(), game,
        {sliderMinX, sliderStartY - sliderSpacing},
        {sliderMaxX, sliderStartY - sliderSpacing},
        {.pegMaterial = game->getMaterial("box")}
    );
    musicSlider->getBar()->setLayer(0.54f);
    musicSlider->getPeg()->setLayer(0.56f);
    musicSlider->setCallback([this](float proportion) {
        // TODO: Set music volume
        std::cout << "Music volume: " << proportion << std::endl;
    });
    settingsMenu->addElement(musicSlider);
    
    // SFX volume slider
    Node2D* sfxIcon = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("man"),
        .position = {iconX, sliderStartY - sliderSpacing * 2},
        .scale = {iconSize, iconSize}
    });
    sfxIcon->setLayer(0.55f);
    settingsMenu->addNode(sfxIcon);
    
    Slider* sfxSlider = new Slider(game->getMenuScene(), game,
        {sliderMinX, sliderStartY - sliderSpacing * 2},
        {sliderMaxX, sliderStartY - sliderSpacing * 2},
        {.pegMaterial = game->getMaterial("box")}
    );
    sfxSlider->getBar()->setLayer(0.54f);
    sfxSlider->getPeg()->setLayer(0.56f);
    sfxSlider->setCallback([this](float proportion) {
        // TODO: Set SFX volume
        std::cout << "SFX volume: " << proportion << std::endl;
    });
    settingsMenu->addElement(sfxSlider);

    // Back button
    Button* backButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {0, -3},
            .scale = {2, 1}
        },
        {
            .onUp = [this]() { 
                this->popMenu();
            }
        }
    );
    backButton->setLayer(0.6f);
    settingsMenu->addElement(backButton);
    
    return settingsMenu;
}
