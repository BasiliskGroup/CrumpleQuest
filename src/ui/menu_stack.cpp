#include "ui/menu_stack.h"
#include "ui/menu.h"

MenuStack::MenuStack() {
}

MenuStack::~MenuStack() {
    clear();
}

void MenuStack::push(Menu* menu) {
    // Hide the current top menu if it exists
    if (!stack.empty()) {
        stack.back()->hide();
    }
    
    // Add new menu and show it
    stack.push_back(menu);
    menu->show();
}

void MenuStack::pop() {
    if (stack.empty()) return;
    
    // Hide and remove top menu
    stack.back()->hide();
    stack.pop_back();
    
    // Show the new top menu if it exists
    if (!stack.empty()) {
        stack.back()->show();
    }
}

void MenuStack::clear() {
    // Hide all menus
    for (auto* menu : stack) {
        menu->hide();
    }
    stack.clear();
}

Menu* MenuStack::top() const {
    if (stack.empty()) return nullptr;
    return stack.back();
}

void MenuStack::handleEvent(const vec2& mousePos, bool mouseDown) {
    if (!stack.empty()) {
        stack.back()->handleEvent(mousePos, mouseDown);
    }
}

void MenuStack::update(float dt) {
    if (!stack.empty()) {
        stack.back()->update(dt);
    }
}
