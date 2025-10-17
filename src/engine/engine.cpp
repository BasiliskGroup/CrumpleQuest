#include "engine/engine.h"

Engine::Engine() : solver(new Solver()), scene(new Scene()) {

}

Engine::~Engine(){
    delete solver;
    delete scene;
}

void Engine::update(float dt){

}
