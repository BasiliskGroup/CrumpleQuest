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
    // Don't push if top menu is animating out
    if (menuStack && menuStack->top() && menuStack->top()->isAnimatingOut()) {
        std::cout << "[MenuManager] Blocked push - menu is animating out" << std::endl;
        return;
    }
    std::cout << "[MenuManager] Creating main menu..." << std::endl;
    Menu* mainMenu = createMainMenu();
    menuStack->push(mainMenu);
}

void MenuManager::pushSettingsMenu() {
    // Don't push if top menu is animating out
    if (menuStack && menuStack->top() && menuStack->top()->isAnimatingOut()) {
        std::cout << "[MenuManager] Blocked push - menu is animating out" << std::endl;
        return;
    }
    Menu* settingsMenu = createSettingsMenu();
    menuStack->push(settingsMenu);
}

void MenuManager::pushGameOverMenu() {
    // Don't push if top menu is animating out
    if (menuStack && menuStack->top() && menuStack->top()->isAnimatingOut()) {
        std::cout << "[MenuManager] Blocked push - menu is animating out" << std::endl;
        return;
    }
    Menu* gameOverMenu = createGameOverMenu();
    menuStack->push(gameOverMenu);
}

void MenuManager::popMenu() {
    if (!menuStack || !menuStack->top()) {
        return;
    }
    Menu* menu = menuStack->top();
    
    // Start the close animation
    menu->startCloseAnimation();
    // Menu will be removed after animation completes in update()
}

void MenuManager::popMenuDeferred() {
    if (!menuStack || !menuStack->top()) {
        return;
    }
    Menu* menu = menuStack->top();
    menuStack->pop();
    pendingDelete.push_back(menu);  // Defer deletion until next frame
}

void MenuManager::deletePendingMenus() {
    for (Menu* menu : pendingDelete) {
        delete menu;
    }
    pendingDelete.clear();
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
    
    // Update menu stack first
    menuStack->update(dt);
    
    // Then check if top menu has finished closing animation
    if (menuStack && menuStack->top()) {
        Menu* topMenu = menuStack->top();
        if (topMenu->isAnimatingOut() && topMenu->isDoneAnimating()) {
            std::cout << "[MenuManager] Menu finished closing animation, removing from stack" << std::endl;
            menuStack->pop();
            delete topMenu;
        }
    }
}

Menu* MenuManager::createMainMenu() {
    Menu* mainMenu = new Menu(game);
    
    // Main menu at layer 0.1-0.3
    // Create background (clickable to play paper touch sound)
    Button* background = new Button(game->getMenuScene(), game, {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("menuPaper"),
        .scale = {7.5, 7.5}  // Keep square scale to preserve menuPaper.PNG aspect ratio (4:3)
    },
    {
        .onDown = [this]() {
            playMenuTouchSound();
        }
    });
    background->setLayer(0.1f);
    mainMenu->addElement(background);

    // Button layout - lower third of paper
    float buttonY = -3.0f;
    float leftButtonSpacing = 2.5f;
    float rightButtonSpacing = 2.2f;
    float buttonWidth = 2.4f;
    float buttonHeight = 3.1f;  // Compensate for 12:9 camera aspect ratio to achieve 1:1 screen appearance
    float buttonOffsetX = 0.1f;  // Slight offset to the right

    // Settings button (left)
    Button* settingsButton = new Button(game->getMenuScene(), game, 
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("settingsButton"),
            .position = {-leftButtonSpacing + buttonOffsetX, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() {
                this->pushSettingsMenu();
            },
            .hoverMaterial = game->getMaterial("settingsButtonHover"),
            .hitboxScale = {buttonWidth, buttonHeight}
        }
    );
    settingsButton->setLayer(0.3f);
    mainMenu->addElement(settingsButton);

    // Start button (center)
    Button* startButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("startButton"),
            .position = {0 + buttonOffsetX, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() {
                // Mark menu for deletion after this callback completes
                Menu* menu = this->menuStack->top();
                this->menuStack->clear();
                this->pendingDelete.push_back(menu);
                // Don't call startGame here - it will be triggered by a flag
                this->game->setPendingStartGame(true);
            },
            .hoverMaterial = game->getMaterial("startButtonHover"),
            .hitboxScale = {buttonWidth, buttonHeight}
        }
    );
    startButton->setLayer(0.3f);
    mainMenu->addElement(startButton);

    // Quit button (right)
    Button* quitButton = new Button(game->getMenuScene(), game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("exitButton"),
            .position = {rightButtonSpacing + buttonOffsetX, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() {
                glfwSetWindowShouldClose(this->game->getEngine()->getWindow()->getWindow(), GLFW_TRUE);
            },
            .hoverMaterial = game->getMaterial("exitButtonHover"),
            .hitboxScale = {buttonWidth, buttonHeight}
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
        .scale = {7.5, 7.5}  // Match main menu dimensions
    },
    {
        .onDown = [this]() {
            this->playMenuTouchSound();
        }
    });
    background->setLayer(0.4f);
    settingsMenu->addElement(background);

    // Sliders layout - centered
    float sliderStartY = 1.3f;
    float sliderSpacing = 0.9f;
    float iconX = -3.0f;
    float sliderMinX = -1.5f;  // Centered slider
    float sliderMaxX = 1.5f;   // Centered slider
    float iconSize = 0.7f;     // Bigger icons
    float pegSize = 0.4f;      // Circle size
    float pegSizeCompensated = pegSize * (12.0f / 9.0f);  // Compensate for aspect ratio to make circles round
    
    // Master volume slider
    Node2D* masterIcon = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("volumeicon"),
        .position = {iconX, sliderStartY},
        .scale = {iconSize, iconSize}
    });
    masterIcon->setLayer(0.55f);
    settingsMenu->addNode(masterIcon);
    
    Slider* masterSlider = new Slider(game->getMenuScene(), game, 
        {sliderMinX, sliderStartY}, 
        {sliderMaxX, sliderStartY}, 
        {
            .pegMaterial = game->getMaterial("blackCircle"),
            .pegDim = {pegSize, pegSizeCompensated}
        }
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
        .material = game->getMaterial("musicicon"),
        .position = {iconX, sliderStartY - sliderSpacing},
        .scale = {iconSize, iconSize}
    });
    musicIcon->setLayer(0.55f);
    settingsMenu->addNode(musicIcon);
    
    Slider* musicSlider = new Slider(game->getMenuScene(), game,
        {sliderMinX, sliderStartY - sliderSpacing},
        {sliderMaxX, sliderStartY - sliderSpacing},
        {
            .pegMaterial = game->getMaterial("blackCircle"),
            .pegDim = {pegSize, pegSizeCompensated}
        }
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
        .material = game->getMaterial("SFXicon"),
        .position = {iconX, sliderStartY - sliderSpacing * 2},
        .scale = {iconSize, iconSize}
    });
    sfxIcon->setLayer(0.55f);
    settingsMenu->addNode(sfxIcon);
    
    Slider* sfxSlider = new Slider(game->getMenuScene(), game,
        {sliderMinX, sliderStartY - sliderSpacing * 2},
        {sliderMaxX, sliderStartY - sliderSpacing * 2},
        {
            .pegMaterial = game->getMaterial("blackCircle"),
            .pegDim = {pegSize, pegSizeCompensated}
        }
    );
    sfxSlider->setProportion(game->getAudio().GetGroupVolume(game->getSFXGroup()) / 0.3f);
    sfxSlider->getBar()->setLayer(0.54f);
    sfxSlider->getPeg()->setLayer(0.56f);
    sfxSlider->setCallback([this](float proportion) {
        // Scale slider to cap maximum volume at 30%
        this->game->getAudio().SetGroupVolume(this->game->getSFXGroup(), proportion * 0.3f);
    });
    settingsMenu->addElement(sfxSlider);

    // Button layout - horizontal like main menu
    float buttonY = -3.0f;
    float buttonWidth = 2.4f;
    float buttonHeight = 3.1f;  // Compensate for 12:9 camera aspect ratio
    
    // Determine button positions based on whether in-game or not
    if (game->getPaper() != nullptr) {
        // In-game: show both Home and Back buttons
        float buttonSpacing = 2.3f;
        
        // Back button (left)
        Button* backButton = new Button(game->getMenuScene(), game,
            {
                .mesh = game->getMesh("quad"),
                .material = game->getMaterial("back"),
                .position = {-buttonSpacing, buttonY},
                .scale = {buttonWidth, buttonHeight}
            },
            {
                .onUp = [this]() {
                    this->popMenu();
                },
                .hoverMaterial = game->getMaterial("back_hover"),
                .hitboxScale = {buttonWidth, buttonHeight}
            }
        );
        backButton->setLayer(0.6f);
        settingsMenu->addElement(backButton);
        
        // Home button (right)
        Button* mainMenuButton = new Button(game->getMenuScene(), game,
            {
                .mesh = game->getMesh("quad"),
                .material = game->getMaterial("home"),
                .position = {buttonSpacing, buttonY},
                .scale = {buttonWidth, buttonHeight}
            },
            {
                .onUp = [this]() {
                    this->game->returnToMainMenu();
                },
                .hoverMaterial = game->getMaterial("home_hover"),
                .hitboxScale = {buttonWidth, buttonHeight}
            }
        );
        mainMenuButton->setLayer(0.6f);
        settingsMenu->addElement(mainMenuButton);
    } else {
        // Main menu settings: only show Back button (centered)
        Button* backButton = new Button(game->getMenuScene(), game,
            {
                .mesh = game->getMesh("quad"),
                .material = game->getMaterial("back"),
                .position = {0, buttonY},
                .scale = {buttonWidth, buttonHeight}
            },
            {
                .onUp = [this]() {
                    this->popMenu();
                },
                .hoverMaterial = game->getMaterial("back_hover"),
                .hitboxScale = {buttonWidth, buttonHeight}
            }
        );
        backButton->setLayer(0.6f);
        settingsMenu->addElement(backButton);
    }
    
    return settingsMenu;
}

Menu* MenuManager::createGameOverMenu() {
    Menu* gameOverMenu = new Menu(game);
    
    // Create background (clickable to play paper touch sound)
    Button* background = new Button(game->getMenuScene(), game, {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("notebook"),
        .scale = {7.5, 7.5}  // Match main menu dimensions
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
            .material = game->getMaterial("home"),
            .position = {0, -1.5},
            .scale = {0.8, 0.8}
        },
        {
            .onUp = [this]() {
                this->game->returnToMainMenu();
            },
            .hoverMaterial = game->getMaterial("home_hover")
        }
    );
    mainMenuButton->setLayer(0.6f);
    gameOverMenu->addElement(mainMenuButton);
    
    return gameOverMenu;
}
