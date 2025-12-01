#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include "util/includes.h"
#include "ui/menu_stack.h"
#include <memory>

class Game;
class Menu;

namespace audio {
    class RandomSoundContainer;
}

class MenuManager {
private:
    Game* game;
    MenuStack* menuStack;
    std::vector<Menu*> pendingDelete; // Menus to delete after current frame
    std::unique_ptr<audio::RandomSoundContainer> touchSoundContainer;
    
    // Helper to play menu touch sound
    void playMenuTouchSound();
    
    // Menu creation functions (menus are created on-demand)
    Menu* createMainMenu();
    Menu* createSettingsMenu();

public:
    MenuManager(Game* game);
    ~MenuManager();

    // Menu navigation
    void pushMainMenu();
    void pushSettingsMenu();
    void popMenu();
    bool hasActiveMenu() const { return menuStack->hasActiveMenu(); }
    
    // Event handling and updates
    void handleEvent(const vec2& mousePos, bool mouseDown);
    void update(float dt);
};

#endif // MENU_MANAGER_H
