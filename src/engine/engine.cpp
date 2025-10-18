#include "engine/engine.h"

Engine::Engine() : scene(new Scene()) {

}

Engine::~Engine(){
    delete scene;
}

void Engine::update(float dt){

}
