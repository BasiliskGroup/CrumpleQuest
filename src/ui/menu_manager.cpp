#include "ui/menu_manager.h"
#include "ui/menu.h"
#include "ui/button.h"
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
    mainMenu->addNode(settingsButton);

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
    mainMenu->addNode(startButton);

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
                // TODO: Quit the game
            }
        }
    );
    quitButton->setLayer(0.3f);
    mainMenu->addNode(quitButton);
    
    return mainMenu;
}

Menu* MenuManager::createSettingsMenu() {
    Menu* settingsMenu = new Menu(game);
    
    // Settings menu at layer 0.4-0.6 (above main menu)
    // Create background
    Node2D* background = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("box"),  // Different material to see it
        .scale = {16, 9}
    });
    background->setLayer(0.4f);
    settingsMenu->addNode(background);
    
    // Create title
    Node2D* title = new Node2D(game->getMenuScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("lightGrey"),  // Different material
        .position = {0, 3},
        .scale = {4, 1.5}
    });
    title->setLayer(0.5f);
    settingsMenu->addNode(title);

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
    settingsMenu->addNode(backButton);
    
    // TODO: Add sliders and other settings controls here
    
    return settingsMenu;
}
