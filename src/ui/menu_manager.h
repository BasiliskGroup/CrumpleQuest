#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "util/includes.h"
#include "ui/menu_stack.h"
#include <memory>

class Game;
class Menu;

class MenuManager {
private:
    Game* game;
    MenuStack* menuStack;
    std::vector<Menu*> pendingDelete; // Menus to delete after current frame
    
    // Private constructor for singleton
    MenuManager();
    
    // Delete copy and move
    MenuManager(const MenuManager&) = delete;
    MenuManager& operator=(const MenuManager&) = delete;
    MenuManager(MenuManager&&) = delete;
    MenuManager& operator=(MenuManager&&) = delete;
    
    // Helper to play menu touch sound
    void playMenuTouchSound();
    
    // Menu creation functions (menus are created on-demand)
    Menu* createMainMenu();
    Menu* createSettingsMenu();
    Menu* createGameOverMenu();

public:
    ~MenuManager();
    
    // Singleton access (Meyers singleton - thread-safe, automatic cleanup)
    static MenuManager& Get();
    
    // Initialize with game instance (must be called before first use)
    void SetGame(Game* game) { this->game = game; }

    // Menu navigation
    void pushMainMenu();
    void pushSettingsMenu();
    void pushGameOverMenu();
    void popMenu();
    void popMenuDeferred(); // Pop menu but defer deletion until next frame
    void deletePendingMenus(); // Immediately delete all pending menus
    bool hasActiveMenu() const { return menuStack->hasActiveMenu(); }
    size_t getMenuStackSize() const { return menuStack->size(); }
    bool isInGame() const;
    
    // Event handling and updates
    void handleEvent(const vec2& mousePos, bool mouseDown);
    void update(float dt);
};

#endif // MENU_MANAGER_H
