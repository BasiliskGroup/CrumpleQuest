#include "util/includes.h"
#include "game/game.h"
#include "ui/ui.h"
#include "levels/levels.h"
#include "audio/audio_manager.h"
#include "ui/menu_manager.h"
#include "resource/animation.h"
#include "resource/animator.h"
#include "weapon/weapon.h"
#include <earcut.hpp>

#include <iostream>
#include "clipper2/clipper.h"

int main() {
    Game* game = new Game();

    // ------------------------------------------
    // Load resources
    // ------------------------------------------

    // image and material
    std::vector<std::string> imageNames = { "man", "paper", "box", "floor", "lightGrey", "test", "knight", "table", "sword", "gun", "bullet", "wand", "green", "red" };
    for (std::string& name : imageNames) {
        game->addImage(name, new Image("textures/" + name + ".png"));
        game->addMaterial(name, new Material({ 1, 1, 1 }, game->getImage(name)));
    }

    // mesh
    std::vector<std::string> meshNames = { "quad", "paper0", "paper1" };
    for (std::string& name : meshNames) game->addMesh(name, new Mesh("models/" + name + ".obj"));

    // ------------------------------------------
    // Load game start
    // ------------------------------------------

    // Setup background music
    auto parchment_track = game->getAudio().CreateTrack();
    game->getAudio().AddLayer(parchment_track, "parchment", "sounds/parchment.wav", "music");
    game->getAudio().SetLayerVolume(parchment_track, "parchment", 1.0f);
    game->getAudio().PlayTrack(parchment_track);

    // load levels
    SingleSide::generateTemplates(game);
    Paper::generateTemplates(game);

    // initialize menus (menu scene is ready)
    game->initMenus();
    game->getMenus()->pushMainMenu();

    while (game->getEngine()->isRunning()) {
        game->update(game->getEngine()->getDeltaTime());
    }
    
    delete game;
}