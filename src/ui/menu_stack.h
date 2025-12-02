#ifndef MENU_STACK_H
#define MENU_STACK_H

#include "util/includes.h"

class Menu;

class MenuStack {
private:
    std::vector<Menu*> stack;
    
public:
    MenuStack();
    ~MenuStack();

    // Stack operations
    void push(Menu* menu);
    void pop();
    void clear();
    
    // Get current menu
    Menu* top() const;
    bool empty() const { return stack.empty(); }
    bool hasActiveMenu() const { return !stack.empty(); }
    
    // Event handling - only passes to top menu
    void handleEvent(const vec2& mousePos, bool mouseDown);
    
    // Update - only updates top menu
    void update(float dt);
};

#endif // MENU_STACK_H
