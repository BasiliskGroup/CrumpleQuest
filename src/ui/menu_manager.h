#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "util/includes.h"
#include "ui/menu_stack.h"

class Game;
class Menu;

class MenuManager {
private:
    Game* game;
    MenuStack* menuStack;
    
    // All menus owned by the manager
    Menu* mainMenu;
    Menu* settingsMenu;
    // Add more menus here as needed

public:
    MenuManager(Game* game);
    ~MenuManager();

    // Menu navigation
    void showMainMenu();
    void showSettingsMenu();
    void popMenu();
    
    // Event handling and updates
    void handleEvent(const vec2& mousePos, bool mouseDown);
    void update(float dt);

private:
    // Menu creation functions
    void createMainMenu();
    void createSettingsMenu();
};

#endif // MENU_MANAGER_H
