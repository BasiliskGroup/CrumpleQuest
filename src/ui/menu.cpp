#include "ui/menu.h"
#include "game/game.h"
#include "ui/slider.h"

Menu::Menu(Game* game) : game(game), visible(true), animationTimer(-1.0f), animationDuration(0.35f), slideDistance(8.0f), animatingOut(false), isClosing(false) {
}

Menu::~Menu() {
    // Clean up all UI elements
    for (auto* element : uiElements) {
        delete element;
    }
    // Clean up all non-UI nodes (images)
    for (auto* node : nodes) {
        delete node;
    }
}

void Menu::addElement(UIElement* element) {
    uiElements.push_back(element);
    
    // Store target Y position for animatable nodes
    // Buttons are Node2D-based, so we can animate them directly
    Node2D* node = dynamic_cast<Node2D*>(element);
    if (node) {
        float originalY = node->getPosition().y;
        targetYPositions[node] = originalY;
    }
    
    // Sliders are not Node2D, but have internal bar and peg nodes we need to animate
    Slider* slider = dynamic_cast<Slider*>(element);
    if (slider) {
        Node2D* bar = slider->getBar();
        Button* peg = slider->getPeg();
        if (bar) {
            targetYPositions[bar] = bar->getPosition().y;
        }
        if (peg) {
            targetYPositions[peg] = peg->getPosition().y;
        }
    }
}

void Menu::addNode(Node2D* node) {
    nodes.push_back(node);
    // Store target Y position
    // Don't offset yet - that happens in resetAnimation() when menu is shown
    float originalY = node->getPosition().y;
    targetYPositions[node] = originalY;  // Store target position for this specific node
}

void Menu::resetAnimation() {
    animationTimer = -999.0f; // Large negative value - will be set to 0 on first update
    animatingOut = false;
    isClosing = false;
    
    std::cout << "[Menu::resetAnimation] Resetting " << uiElements.size() << " elements and " << nodes.size() << " nodes" << std::endl;
    
    // Reset all elements to their starting positions (offset down by slideDistance)
    for (size_t i = 0; i < uiElements.size(); i++) {
        // Handle Button nodes
        Node2D* node = dynamic_cast<Node2D*>(uiElements[i]);
        if (node && targetYPositions.count(node) > 0) {
            float originalY = targetYPositions[node];
            node->setPosition({node->getPosition().x, originalY - slideDistance});
        }
        
        // Handle Slider internal nodes
        Slider* slider = dynamic_cast<Slider*>(uiElements[i]);
        if (slider) {
            Node2D* bar = slider->getBar();
            Button* peg = slider->getPeg();
            if (bar && targetYPositions.count(bar) > 0) {
                float originalY = targetYPositions[bar];
                bar->setPosition({bar->getPosition().x, originalY - slideDistance});
            }
            if (peg && targetYPositions.count(peg) > 0) {
                float originalY = targetYPositions[peg];
                peg->setPosition({peg->getPosition().x, originalY - slideDistance});
            }
        }
    }
    
    for (size_t i = 0; i < nodes.size(); i++) {
        if (targetYPositions.count(nodes[i]) > 0) {
            float originalY = targetYPositions[nodes[i]];
            nodes[i]->setPosition({nodes[i]->getPosition().x, originalY - slideDistance});
        }
    }
}

void Menu::startCloseAnimation() {
    animationTimer = 0.0f;
    animatingOut = true;
    isClosing = true;
    std::cout << "[Menu::startCloseAnimation] Starting slide-out animation" << std::endl;
}

void Menu::update(float dt) {
    // Start animation on first update call if it hasn't started yet
    if (animationTimer < -100.0f && !animatingOut) {
        std::cout << "[Menu::update] Starting slide-in animation" << std::endl;
        animationTimer = 0.0f;
    }
    
    // Animate menu sliding in or out
    if (animationTimer < animationDuration) {
        animationTimer += dt;
        
        static int frameCount = 0;
        if (frameCount++ < 3) {
            std::cout << "[Menu::update] Animation frame " << frameCount << " - timer: " << animationTimer 
                      << " (animating " << (animatingOut ? "OUT" : "IN") << ")" << std::endl;
        }
        
        // Calculate animation progress (0 to 1)
        float progress = std::min(animationTimer / animationDuration, 1.0f);
        
        // Animate UI elements
        for (size_t i = 0; i < uiElements.size(); i++) {
            // Animate Button nodes
            Node2D* node = dynamic_cast<Node2D*>(uiElements[i]);
            if (node && targetYPositions.count(node) > 0) {
                float targetY = targetYPositions[node];
                float startY = targetY - slideDistance;
                // If animating out, reverse the animation
                float newY = animatingOut ? (targetY - progress * slideDistance) : (startY + progress * slideDistance);
                node->setPosition({node->getPosition().x, newY});
            }
            
            // Animate Slider internal nodes
            Slider* slider = dynamic_cast<Slider*>(uiElements[i]);
            if (slider) {
                Node2D* bar = slider->getBar();
                Button* peg = slider->getPeg();
                if (bar && targetYPositions.count(bar) > 0) {
                    float targetY = targetYPositions[bar];
                    float startY = targetY - slideDistance;
                    float newY = animatingOut ? (targetY - progress * slideDistance) : (startY + progress * slideDistance);
                    bar->setPosition({bar->getPosition().x, newY});
                }
                if (peg && targetYPositions.count(peg) > 0) {
                    float targetY = targetYPositions[peg];
                    float startY = targetY - slideDistance;
                    float newY = animatingOut ? (targetY - progress * slideDistance) : (startY + progress * slideDistance);
                    peg->setPosition({peg->getPosition().x, newY});
                }
            }
        }
        
        // Animate plain nodes
        for (size_t i = 0; i < nodes.size(); i++) {
            if (targetYPositions.count(nodes[i]) > 0) {
                float targetY = targetYPositions[nodes[i]];
                float startY = targetY - slideDistance;
                float newY = animatingOut ? (targetY - progress * slideDistance) : (startY + progress * slideDistance);
                nodes[i]->setPosition({nodes[i]->getPosition().x, newY});
            }
        }
    } else {
        // Animation complete - ensure elements are at final positions
        static bool snapPrinted = false;
        if (!snapPrinted) {
            for (size_t i = 0; i < uiElements.size(); i++) {
                // Snap Button nodes
                Node2D* node = dynamic_cast<Node2D*>(uiElements[i]);
                if (node && targetYPositions.count(node) > 0) {
                    float targetY = targetYPositions[node];
                    node->setPosition({node->getPosition().x, targetY});
                }
                
                // Snap Slider internal nodes
                Slider* slider = dynamic_cast<Slider*>(uiElements[i]);
                if (slider) {
                    Node2D* bar = slider->getBar();
                    Button* peg = slider->getPeg();
                    if (bar && targetYPositions.count(bar) > 0) {
                        float targetY = targetYPositions[bar];
                        bar->setPosition({bar->getPosition().x, targetY});
                    }
                    if (peg && targetYPositions.count(peg) > 0) {
                        float targetY = targetYPositions[peg];
                        peg->setPosition({peg->getPosition().x, targetY});
                    }
                }
            }
            
            for (size_t i = 0; i < nodes.size(); i++) {
                if (targetYPositions.count(nodes[i]) > 0) {
                    float targetY = targetYPositions[nodes[i]];
                    nodes[i]->setPosition({nodes[i]->getPosition().x, targetY});
                }
            }
            snapPrinted = true;
        }
    }
}

void Menu::handleEvent(const vec2& mousePos, bool mouseDown) {
    if (!visible) return;
    
    // Pass events to all UI elements
    for (UIElement* element : uiElements) {
        element->event(mousePos, mouseDown);
    }
}