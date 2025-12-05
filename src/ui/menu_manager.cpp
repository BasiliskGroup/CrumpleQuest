#include "ui/menu_manager.h"
#include "ui/menu.h"
#include "ui/button.h"
#include "ui/slider.h"
#include "game/game.h"
#include <iostream>

MenuManager::MenuManager() : game(nullptr) {
    menuStack = new MenuStack();
}

MenuManager::~MenuManager() {
    delete menuStack;
}

MenuManager& MenuManager::Get() {
    static MenuManager instance; // Meyers singleton - thread-safe in C++11+
    return instance;
}

void MenuManager::playMenuTouchSound() {
    audio::SFXPlayer::Get().Play("menu_touch");
}

void MenuManager::pushMainMenu() {
    Menu* mainMenu = createMainMenu();
    menuStack->push(mainMenu);
}

void MenuManager::pushSettingsMenu() {
    Menu* settingsMenu = createSettingsMenu();
    menuStack->push(settingsMenu);
}

void MenuManager::pushGameOverMenu() {
    Menu* gameOverMenu = createGameOverMenu();
    menuStack->push(gameOverMenu);
}

void MenuManager::popMenu() {
    if (!menuStack || !menuStack->top()) {
        return;
    }
    Menu* menu = menuStack->top();
    menuStack->pop();
    delete menu;  // Delete the menu when hiding it
}

void MenuManager::popMenuDeferred() {
    if (!menuStack || !menuStack->top()) {
        return;
    }
    Menu* menu = menuStack->top();
    menuStack->pop();
    pendingDelete.push_back(menu);  // Defer deletion until next frame
}

bool MenuManager::isInGame() const {
    return game && game->getPaper() != nullptr;
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
    // Create background (clickable to play paper touch sound)
    Button* background = new Button(game->getMenuScene(), game, {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("notebook"),
        .scale = {9, 6.75}
    },
    {
        .onDown = [this]() {
            playMenuTouchSound();
        }
    });
    background->setLayer(0.1f);
    mainMenu->addElement(background);
    
    // Create title
    Node2D* title = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("paper"),
        .position = {0, 2.2},
        .scale = {4, 1.2}
    });
    title->setLayer(0.2f);
    mainMenu->addNode(title);

    // Button layout
    float buttonY = -0.2f;
    float buttonSpacing = 2.0f;
    float buttonWidth = 1.5f;
    float buttonHeight = 0.8f;

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
    
    // Create background (clickable to play paper touch sound)
    Button* background = new Button(game->getMenuScene(), game, {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("notebook"),
        .scale = {9, 6.75}
    },
    {
        .onDown = [this]() {
            this->playMenuTouchSound();
        }
    });
    background->setLayer(0.4f);
    settingsMenu->addElement(background);
    
    // Create title
    Node2D* title = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("lightGrey"),
        .position = {0, 2.4},
        .scale = {3.5, 1.2}
    });
    title->setLayer(0.5f);
    settingsMenu->addNode(title);

    // Sliders layout
    float sliderStartY = 1.3f;
    float sliderSpacing = 0.9f;
    float iconX = -3.2f;
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
    masterSlider->setProportion(game->getAudio().GetMasterVolume());
    masterSlider->getBar()->setLayer(0.54f);
    masterSlider->getPeg()->setLayer(0.56f);
    masterSlider->setCallback([this](float proportion) {
        this->game->getAudio().SetMasterVolume(proportion);
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
    musicSlider->setProportion(game->getAudio().GetGroupVolume(game->getMusicGroup()));
    musicSlider->getBar()->setLayer(0.54f);
    musicSlider->getPeg()->setLayer(0.56f);
    musicSlider->setCallback([this](float proportion) {
        this->game->getAudio().SetGroupVolume(this->game->getMusicGroup(), proportion);
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
    sfxSlider->setProportion(game->getAudio().GetGroupVolume(game->getSFXGroup()) / 0.3f);
    sfxSlider->getBar()->setLayer(0.54f);
    sfxSlider->getPeg()->setLayer(0.56f);
    sfxSlider->setCallback([this](float proportion) {
        // Scale slider to cap maximum volume at 30%
        this->game->getAudio().SetGroupVolume(this->game->getSFXGroup(), proportion * 0.3f);
    });
    settingsMenu->addElement(sfxSlider);

    // Return to Main Menu button (only shown when in-game)
    if (game->getPaper() != nullptr) {
        Button* mainMenuButton = new Button(game->getMenuScene(), game,
            {
                .mesh = game->getMesh("quad"),
                .material = game->getMaterial("lightGrey"),
                .position = {0, -1.5},
                .scale = {2.8, 0.8}
            },
            {
                .onUp = [this]() {
                    this->game->returnToMainMenu();
                }
            }
        );
        mainMenuButton->setLayer(0.6f);
        settingsMenu->addElement(mainMenuButton);
    }

    // Back button
    Button* backButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {0, game->getPaper() != nullptr ? -2.4f : -2.2},
            .scale = {2.0, 0.8}
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

Menu* MenuManager::createGameOverMenu() {
    Menu* gameOverMenu = new Menu(game);
    
    // Create background (clickable to play paper touch sound)
    Button* background = new Button(game->getMenuScene(), game, {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("notebook"),
        .scale = {9, 6.75}
    },
    {
        .onDown = [this]() {
            this->playMenuTouchSound();
        }
    });
    background->setLayer(0.4f);
    gameOverMenu->addElement(background);
    
    // Create game over image/title
    Node2D* gameOverImage = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("lightGrey"),
        .position = {0, 1.0},
        .scale = {4, 2}
    });
    gameOverImage->setLayer(0.5f);
    gameOverMenu->addNode(gameOverImage);
    
    // Return to Main Menu button
    Button* mainMenuButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {0, -1.5},
            .scale = {2.8, 0.8}
        },
        {
            .onUp = [this]() {
                this->game->returnToMainMenu();
            }
        }
    );
    mainMenuButton->setLayer(0.6f);
    gameOverMenu->addElement(mainMenuButton);
    
    return gameOverMenu;
}
