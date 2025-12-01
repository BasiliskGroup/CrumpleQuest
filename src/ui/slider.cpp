#include "ui/ui.h"

Slider::Slider(Scene2D* scene, Game* game, const vec2& start, const vec2& end, Params params) :
    game(game),
    start(start),
    end(end)
{
    if (end.x <= start.x) throw std::invalid_argument("Slider: End must come after start.");
    this->callback = params.callback;

    this->end.y = this->start.y; // I dont want to do math
    Material* deflt = game->getMaterial("lightGrey");

    // compute node geo 
    vec2 mid = (start + end) / 2.0f;
    float width = end.x - start.x;

    // peg
    peg = new Button(scene, game, { 
        .mesh = params.pegMesh ? params.pegMesh : game->getMesh("quad"), 
        .material = params.pegMaterial ? params.pegMaterial : deflt,
        .scale = params.pegDim
    }, {
        .onDown = [this]() {
            this->holding = true;
        }
    });
    peg->setLayer(0.91);
    setProportion(params.startProportion);

    // slider bar
    bar = new Node2D(scene, { 
        .mesh = game->getMesh("quad"),
        .material = params.barMaterial ? params.barMaterial : deflt,
        .position = mid,
        .scale = { width, params.barHeight }
    });
}

Slider::~Slider() {
    delete bar;
    delete peg;
}

// Copy constructor
Slider::Slider(const Slider& other) :
    game(other.game),
    callback(other.callback),
    start(other.start),
    end(other.end),
    startDot(other.startDot),
    endDot(other.endDot),
    dir(other.dir),
    holding(false)  // Don't copy holding state
{
    // Deep copy bar
    bar = new Node2D(*other.bar);
    
    // Deep copy peg (need to re-establish lambda capture)
    peg = new Button(*other.peg);
    peg->setOnDown([this]() {
        this->holding = true;
    });
}

// Copy assignment operator
Slider& Slider::operator=(const Slider& other) {
    if (this != &other) {
        // Clean up existing resources
        delete bar;
        delete peg;
        
        // Copy data members
        game = other.game;
        callback = other.callback;
        start = other.start;
        end = other.end;
        startDot = other.startDot;
        endDot = other.endDot;
        dir = other.dir;
        holding = false;  // Don't copy holding state
        
        // Deep copy bar
        bar = new Node2D(*other.bar);
        
        // Deep copy peg (need to re-establish lambda capture)
        peg = new Button(*other.peg);
        peg->setOnDown([this]() {
            this->holding = true;
        });
    }
    return *this;
}

// Move constructor
Slider::Slider(Slider&& other) noexcept :
    game(other.game),
    bar(other.bar),
    peg(other.peg),
    callback(std::move(other.callback)),
    start(other.start),
    end(other.end),
    startDot(other.startDot),
    endDot(other.endDot),
    dir(other.dir),
    holding(other.holding)
{
    // Nullify other's pointers
    other.bar = nullptr;
    other.peg = nullptr;
    
    // Update lambda capture to point to this instance
    if (peg) {
        peg->setOnDown([this]() {
            this->holding = true;
        });
    }
}

// Move assignment operator
Slider& Slider::operator=(Slider&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        delete bar;
        delete peg;
        
        // Move data members
        game = other.game;
        bar = other.bar;
        peg = other.peg;
        callback = std::move(other.callback);
        start = other.start;
        end = other.end;
        startDot = other.startDot;
        endDot = other.endDot;
        dir = other.dir;
        holding = other.holding;
        
        // Nullify other's pointers
        other.bar = nullptr;
        other.peg = nullptr;
        
        // Update lambda capture to point to this instance
        if (peg) {
            peg->setOnDown([this]() {
                this->holding = true;
            });
        }
    }
    return *this;
}

void Slider::event(const vec2& mousePos, bool mouseDown) {
    peg->event(mousePos, mouseDown);
    if (mouseDown == false) {
        holding = false;
        return;
    }

    if (holding == false) return;
    
    // move slider to correct position
    vec2 dir = glm::normalize(end - start);
    vec2 toMouse = mousePos - start;
    float p = glm::dot(toMouse, dir);
    p = glm::clamp(p, 0.0f, end.x - start.x);
    peg->setPosition(start + dir * p);

    // update slider function
    callback(getProportion());
}

float Slider::getProportion() {
    computeBoundGeometry();
    return (glm::dot(peg->getPosition(), dir) - startDot) / (endDot - startDot);
}

void Slider::setProportion(float proportion) {
    proportion = glm::clamp(proportion, 0.0f, 1.0f);
    computeBoundGeometry();
    peg->setPosition(start + proportion * dir);
}

void Slider::computeBoundGeometry() {
    dir = end - start;
    startDot = glm::dot(start, dir);
    endDot = glm::dot(end, dir);
}