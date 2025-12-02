#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "util/includes.h"

class UIElement {
public:
    virtual ~UIElement() = default;
    virtual void event(const vec2& pos, bool mouseDown) = 0;
};

#endif