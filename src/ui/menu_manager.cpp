#include "ui/menu_manager.h"
#include "ui/menu.h"
#include "ui/button.h"
#include "game/game.h"
#include <iostream>

MenuManager::MenuManager(Game* game) : game(game), mainMenu(nullptr), settingsMenu(nullptr) {
    menuStack = new MenuStack();
    
    // Create all menus
    createMainMenu();
    createSettingsMenu();
}

MenuManager::~MenuManager() {
    delete mainMenu;
    delete settingsMenu;
    delete menuStack;
}

void MenuManager::showMainMenu() {
    menuStack->push(mainMenu);
}

void MenuManager::showSettingsMenu() {
    menuStack->push(settingsMenu);
}

void MenuManager::popMenu() {
    menuStack->pop();
}

void MenuManager::handleEvent(const vec2& mousePos, bool mouseDown) {
    menuStack->handleEvent(mousePos, mouseDown);
}

void MenuManager::update(float dt) {
    menuStack->update(dt);
}

void MenuManager::createMainMenu() {
    mainMenu = new Menu(game);
    
    // Create title
    Node2D* title = new Node2D(game->getScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("paper"), // Replace with your title material
        .position = {0, 3},
        .scale = {6, 2}
    });
    title->setLayer(0.95);
    mainMenu->addNode(title);

    // Button layout
    float buttonY = 0.5f;
    float buttonSpacing = 3.0f;
    float buttonWidth = 2.0f;
    float buttonHeight = 1.0f;

    // Settings button (left)
    Button* settingsButton = new Button(game, 
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {-buttonSpacing, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() { 
                std::cout << "Settings clicked" << std::endl;
                this->showSettingsMenu();
            }
        }
    );
    mainMenu->addInteractive(settingsButton);

    // Start button (center)
    Button* startButton = new Button(game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("lightGrey"),
            .position = {0, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() { 
                std::cout << "Start clicked" << std::endl;
                // TODO: Start the game
            }
        }
    );
    mainMenu->addInteractive(startButton);

    // Quit button (right)
    Button* quitButton = new Button(game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("man"),
            .position = {buttonSpacing, buttonY},
            .scale = {buttonWidth, buttonHeight}
        },
        {
            .onUp = [this]() { 
                std::cout << "Quit clicked" << std::endl;
                // TODO: Quit the game
            }
        }
    );
    mainMenu->addInteractive(quitButton);
}

void MenuManager::createSettingsMenu() {
    settingsMenu = new Menu(game);
    
    // Create title
    Node2D* title = new Node2D(game->getScene(), {
        .mesh = game->getMesh("quad"),
        .material = game->getMaterial("paper"),
        .position = {0, 3},
        .scale = {4, 1.5}
    });
    title->setLayer(0.95);
    settingsMenu->addNode(title);

    // Back button
    Button* backButton = new Button(game,
        {
            .mesh = game->getMesh("quad"),
            .material = game->getMaterial("box"),
            .position = {0, -3},
            .scale = {2, 1}
        },
        {
            .onUp = [this]() { 
                std::cout << "Back clicked" << std::endl;
                this->popMenu();
            }
        }
    );
    settingsMenu->addInteractive(backButton);
    
    // TODO: Add sliders and other settings controls here
}
