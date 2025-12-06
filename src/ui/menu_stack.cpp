#include "ui/menu_stack.h"
#include "ui/menu.h"

MenuStack::MenuStack() {
}

MenuStack::~MenuStack() {
    clear();
}

void MenuStack::push(Menu* menu) {
    stack.push_back(menu);
    menu->resetAnimation(); // Start slide-in animation when menu becomes visible
}

void MenuStack::pop() {
    if (stack.empty()) return;
    
    // caller handles deletion
    stack.pop_back();
}

void MenuStack::clear() {
    // caller handles deletion
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
